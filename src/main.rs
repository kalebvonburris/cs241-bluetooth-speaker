#![no_std]
#![no_main]
// Technically, making a static mutable reference is
// undefined behavior, but I said I want to, so too bad!
#![allow(static_mut_refs)]

mod led_logic;
use led_logic::*;

use cortex_m_rt::entry;
use rp2040_hal::{
    clocks::{init_clocks_and_plls, Clock},
    gpio::{FunctionPio0, FunctionSioInput, Interrupt::EdgeLow, Pin, Pins, PullUp},
    pac::{self, interrupt},
    pio::PIOExt,
    sio::Sio,
    timer::Timer,
    watchdog::Watchdog,
};

use rp2040_flash::flash;

use smart_leds::{SmartLedsWrite, RGB8};
use ws2812_pio::Ws2812;

use core::cell::RefCell;
use cortex_m::interrupt::{free, Mutex};

/// Bootloader configuration, placed at the correct memory location.
/// This is REQUITED for the RP2040 to boot correctly. Otherwise,
/// we only have the initial bootloader in ROM, which is very limited.
#[link_section = ".boot2"]
#[used]
pub static BOOT2: [u8; 256] = rp2040_boot2::BOOT_LOADER_GENERIC_03H;

/// Mutex to track button press state.
static BUTTON_PRESSED: Mutex<RefCell<bool>> = Mutex::new(RefCell::new(false));

/// The actual button pin used in the interrupt handler.
static mut BUTTON_PIN: Option<Pin<rp2040_hal::gpio::bank0::Gpio3, FunctionSioInput, PullUp>> = None;

/// The frequency of the external crystal oscillator.
const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// There used to be panic handler code here, but it's unnecessary
/// as my code would never panic. :)
#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {
        cortex_m::asm::wfi();
    }
}

/// Interrupt handler for the button press.
///
/// When we press the button during the sleep state,
/// this interrupt will be triggered, waking the microcontroller.
#[interrupt]
fn IO_IRQ_BANK0() {
    free(|cs| {
        BUTTON_PRESSED.borrow(cs).replace(true);
    });

    // Clear the interrupt
    unsafe {
        if let Some(ref mut pin) = BUTTON_PIN.as_mut() {
            pin.clear_interrupt(EdgeLow);
        }
    }
}

/// The offset in flash memory where we store the counter.
/// This is the end of the flash ROM area (2MB) minus a 4KB block.
const FLASH_COUNTER_OFFSET: u32 = (2 * 1024 * 1024) - 4096;
/// The XIP memory base address.
const XIP_BASE: u32 = 0x10000000;

/// Reads a `u64` from the last block of flash memory.
fn read_counter_from_flash() -> u64 {
    let flash_addr = (XIP_BASE + FLASH_COUNTER_OFFSET) as *const u64;
    let value = unsafe { core::ptr::read_volatile(flash_addr) };
    if value == 0xFFFFFFFFFFFFFFFF {
        0
    } else {
        value
    }
}

/// Erases and writes a `u64` value to the last block of flash memory.
fn write_counter_to_flash(value: u64) {
    let mut buffer = [0xFFu8; 4096];
    buffer[0..8].copy_from_slice(&value.to_le_bytes());
    // This stops any interrupts during flash operations
    cortex_m::interrupt::free(|_| unsafe {
        flash::flash_range_erase_and_program(FLASH_COUNTER_OFFSET, &buffer, false);
    });
}

#[entry]
fn main() -> ! {
    let mut pac = pac::Peripherals::take().unwrap();
    let core = pac::CorePeripherals::take().unwrap();
    let sio = Sio::new(pac.SIO);
    let mut watchdog = Watchdog::new(pac.WATCHDOG);

    let clocks = init_clocks_and_plls(
        XTAL_FREQ_HZ,
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut watchdog,
    )
    .ok()
    .unwrap();

    let mut delay = cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz());
    let timer = Timer::new(pac.TIMER, &mut pac.RESETS, &clocks);

    let pins = Pins::new(
        pac.IO_BANK0,
        pac.PADS_BANK0,
        sio.gpio_bank0,
        &mut pac.RESETS,
    );

    let (mut pio, sm0, _, _, _) = pac.PIO0.split(&mut pac.RESETS);
    let mut ws = Ws2812::new(
        pins.gpio2.into_function::<FunctionPio0>(),
        &mut pio,
        sm0,
        clocks.peripheral_clock.freq(),
        timer.count_down(),
    );

    // READ COUNTER ON BOOT
    let mut counter: u64 = read_counter_from_flash();

    // Setup interrupt button
    let interrupt_pin = pins.gpio3.into_pull_up_input();
    interrupt_pin.set_interrupt_enabled(EdgeLow, true);

    // Enable GPIO interrupt
    unsafe {
        BUTTON_PIN = Some(interrupt_pin);
        pac::NVIC::unmask(pac::Interrupt::IO_IRQ_BANK0);
    }

    // Button configuration
    let mut button_was_pressed = false;

    // Setup activity timing
    let mut last_activity = timer.get_counter();

    let blank = [RGB8::default(); WIDTH * HEIGHT];
    let _ = ws.write(blank);

    loop {
        // Read button (low when pressed due to pull-up)
        let button_pressed = free(|cs| BUTTON_PRESSED.borrow(cs).replace(false));

        // If we just pressed the button, increment counter and update display
        if button_pressed && !button_was_pressed {
            counter += 1;

            if counter > 99_999_999 {
                counter = 0;
            }

            let leds = logical_array_to_zig_zag(num_to_pixels(counter));

            let _ = ws.write(leds);

            last_activity = timer.get_counter();
        }

        button_was_pressed = button_pressed;

        // Idle condition - more than 10 seconds of inactivity
        if (timer.get_counter().duration_since_epoch() - last_activity.duration_since_epoch())
            .to_secs()
            > 10
        {
            // Write counter to flash
            write_counter_to_flash(counter);

            // Clear LED grid
            let _ = ws.write(blank);

            //
            loop {
                cortex_m::asm::wfi();

                let awoken_by_button = free(|cs| BUTTON_PRESSED.borrow(cs).replace(false));

                // Wake up, increment, display, and reset activity timer
                if awoken_by_button {
                    counter += 1;

                    if counter > 99_999_999 {
                        counter = 0;
                    }

                    let _ = ws.write(logical_array_to_zig_zag(num_to_pixels(counter)));
                    last_activity = timer.get_counter();
                    delay.delay_ms(100);
                    button_was_pressed = true;
                    break;
                }
            }
        }

        delay.delay_ms(30);
    }
}
