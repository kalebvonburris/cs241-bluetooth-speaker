//! # LED Logic
//!
//! This module contains logic for converting numbers into pixel data
//! and mapping that data to a zig-zag wired 8x32 LED panel.

use smart_leds::RGB8;

/// The height of each strip of LEDs.
pub const HEIGHT: usize = 8;
/// The width/column count of the LED panel.
pub const WIDTH: usize = 32;
/// Output RGB brightness value.
/// This can be a lot higher if a resistor is applied
/// between the LED strip VCC and battery/ground.
const BRIGHTNESS: u8 = 10;

/// The number of digits able to be displayed left to right.
/// Should be that `NUM_DIGITS|WIDTH`, that it evenly divides.
const NUM_DIGITS: usize = 8;

/// The colors for each digit index.
/// A gradient from purple to red.
const DIGIT_INDEX_COLORS: &[RGB8; NUM_DIGITS] = &[
    RGB8 {
        // Leftmost digit - Purple
        r: 255,
        g: 223,
        b: 0,
    },
    RGB8 {
        r: 127,
        g: 0,
        b: 84,
    },
    RGB8 {
        r: 255,
        g: 10,
        b: 100,
    },
    RGB8 {
        r: 127,
        g: 32,
        b: 5,
    },
    RGB8 {
        r: 127,
        g: 64,
        b: 0,
    },
    RGB8 { r: 0, g: 0, b: 255 },
    RGB8 { r: 0, g: 127, b: 0 },
    RGB8 {
        // Rightmost digit - Red
        r: 255,
        g: 0,
        b: 0,
    },
];

/// 4x4 bitmap font for digits `0`-`9`.
pub const DIGITS: &[&[u8; 8]] = &[
    &[
        0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110,
    ], // 0
    &[
        0b0010, 0b1110, 0b0010, 0b0010, 0b0010, 0b0010, 0b0010, 0b1111,
    ], // 1
    &[
        0b0110, 0b1001, 0b0001, 0b0010, 0b0100, 0b1000, 0b1000, 0b1111,
    ], // 2
    &[
        0b0110, 0b1001, 0b0001, 0b0010, 0b0001, 0b0001, 0b1001, 0b0110,
    ], // 3
    &[
        0b1001, 0b1001, 0b1001, 0b1111, 0b0001, 0b0001, 0b0001, 0b0001,
    ], // 4
    &[
        0b1111, 0b1000, 0b1000, 0b1110, 0b0001, 0b0001, 0b1001, 0b0110,
    ], // 5
    &[
        0b0110, 0b1001, 0b1000, 0b1110, 0b1001, 0b1001, 0b1001, 0b0110,
    ], // 6
    &[
        0b1111, 0b0001, 0b0010, 0b0010, 0b0100, 0b0100, 0b0100, 0b0100,
    ], // 7
    &[
        0b0110, 0b1001, 0b1001, 0b0110, 0b1001, 0b1001, 0b1001, 0b0110,
    ], // 8
    &[
        0b00110, 0b1001, 0b1001, 0b0111, 0b0001, 0b0001, 0b1001, 0b0110,
    ], // 9
];

/// Takes a given `u64` number (0-999,999) and coverts it into an array of RGB8 pixel values.
/// At the moment it defaults to red digits on a blank background.
pub fn num_to_pixels(mut num: u64) -> [RGB8; WIDTH * HEIGHT] {
    let mut values = [RGB8::default(); WIDTH * HEIGHT];
    let mut index = NUM_DIGITS - 1;

    while num != 0 {
        let digit = (num % 10) as usize;
        num /= 10;

        let digit_bitmap = DIGITS[digit];

        for (row, &row_bits) in digit_bitmap.iter().enumerate() {
            for col in 0..=3 {
                if (row_bits >> (3 - col)) & 1 == 1 {
                    let x_padding = index * 4;
                    let x = x_padding + col;
                    let y = row;
                    values[y * WIDTH + x] = DIGIT_INDEX_COLORS[index] / 4;
                }
            }
        }

        if index == 0 {
            break;
        }

        index -= 1;
    }

    values
}

/// The LED array is an 8x32 grid, but is wired like this:
/// S = Start, E = End
/// ```
/// =============================
/// S  -> ...32 total columns...V
/// |  ^                        |
/// V  |                        V
/// |  |                        |
/// V  ^                        V
/// |  |                        |
/// V  ^                        V
/// -> |                        E
/// =============================
/// ```
///
/// So  we start at S, encode the positions in a zig-zag manner down, then over, then up,
/// then over, etc.
pub fn logical_array_to_zig_zag(arr: [RGB8; WIDTH * HEIGHT]) -> [RGB8; WIDTH * HEIGHT] {
    let mut output = [RGB8::default(); WIDTH * HEIGHT];

    for (pos, val) in arr.iter().enumerate() {
        let col = pos % WIDTH; // Column (strip number)
        let row = pos / WIDTH; // Row within that column

        let pixel_index = if col.is_multiple_of(2) {
            // Even columns: normal order (top to bottom)
            col * HEIGHT + row
        } else {
            // Odd columns: reversed order (bottom to top)
            col * HEIGHT + (HEIGHT - 1 - row)
        }
        .min(WIDTH * HEIGHT - 1);

        output[pixel_index] = *val;
    }

    output
}
