extern crate sdl2;

use sdl2::event::Event;
use sdl2::gfx::primitives::DrawRenderer;
use sdl2::keyboard::Keycode;
use sdl2::pixels::Color;
use std::time::Duration;

extern crate rand;
use rand::Rng;

const SPEED: i16 = 10;
const RADIUS: i16 = 10;

const SCREEN_WIDTH: i16 = 500;
const SCREEN_HEIGHT: i16 = 500;

struct Point {
    //usually I'd use a tuple but I want it to be similar to the C version
    pub x: i16,
    pub y: i16,
}

struct Ball {
    pub pos: Point,
    pub vel: Point,
}

pub fn main() {
    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let window = video_subsystem
        .window("brickbreaker", SCREEN_WIDTH as u32, SCREEN_HEIGHT as u32)
        .position_centered()
        .build()
        .unwrap();

    let mut canvas = window.into_canvas().build().unwrap();

    let mut rng = rand::thread_rng();

    let mut ball = Ball {
        pos: Point {
            x: SCREEN_WIDTH / 2,
            y: SCREEN_HEIGHT / 2,
        },
        vel: Point {
            x: rng.gen_range(0, SPEED) - SPEED / 2,
            y: rng.gen_range(0, SPEED) - SPEED / 2,
        },
    };

    canvas.set_draw_color(Color::RGB(0, 0, 0));
    canvas.clear();
    canvas.present();
    let mut event_pump = sdl_context.event_pump().unwrap();
    'running: loop {
        canvas.set_draw_color(Color::RGB(0, 0, 0));
        canvas.clear();
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => break 'running,
                _ => {}
            }
        }

        if ball.pos.y + RADIUS / 2 >= SCREEN_HEIGHT || ball.pos.y - RADIUS / 2 <= 0 {
            ball.vel.y *= -1;
        }

        canvas.filled_circle::<Color>(ball.pos.x, ball.pos.y, RADIUS, Color::RGB(255, 255, 255)).unwrap();


        ball.pos.x += ball.vel.x;
        ball.pos.y += ball.vel.y;

        canvas.present();
        ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 60));
    }
}
