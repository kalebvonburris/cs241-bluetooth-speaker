#![no_std]
#![no_main]

use defmt_rtt as _;
use embassy_executor::Spawner;
use embassy_rp::gpio::{Level, Output};
use embassy_time::{Duration, Timer};
use panic_probe as _;

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    let peripherals = embassy_rp::init(Default::default());
    let mut led = Output::new(peripherals.PIN_2, Level::Low);
    loop {
        led.set_high();
        Timer::after(Duration::from_millis(500)).await;

        led.set_low();
        Timer::after(Duration::from_millis(500)).await;
    }
}
