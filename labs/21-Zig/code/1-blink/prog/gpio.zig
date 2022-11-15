extern fn GET32(addr: *volatile u32) u32;
extern fn PUT32(addr: *volatile u32, val: u32) void;
// see broadcomm documents for magic addresses.
const GPIO_BASE = @intToPtr(*volatile u32, 0x20200000);
const GPIO_BASE_INT = @ptrToInt(GPIO_BASE);
const gpio_set0 = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x1C);
const gpio_clr0 = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x28);
const gpio_lev0 = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x34);
const gppud = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x94);
const gppud_clk0 = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x98);
const gppud_clk1 = @intToPtr(*volatile u32, GPIO_BASE_INT + 0x9c);

// from broadcom
const GpioFunc = enum(u32) {
    GPIO_FUNC_INPUT = 0,
    GPIO_FUNC_OUTPUT = 1,
    GPIO_FUNC_ALT0 = 4,
    GPIO_FUNC_ALT1 = 5,
    GPIO_FUNC_ALT2 = 6,
    GPIO_FUNC_ALT3 = 7,
    GPIO_FUNC_ALT4 = 3,
    GPIO_FUNC_ALT5 = 2,
};

// Just some ideas
const GpioError = error{
    InvalidPin,
    InvalidFunction,
};

const GpioState = enum(u8) {
    off = 0,
    on = 1,
};

pub const Pin = struct {
    num: u32,
    func: GpioFunc = undefined,
};

// TODO : Write your GPIO implementation here. I would recommend closely following the code we wrote in 140e,
// but dont be afraid to use Zig features like comptime, error unions, etc. 
// You can choose to either make a Gpio struct with member functions or just make regular public functions. 
// Pick whichever way you like and try it, but youll probably want to comment out the other.
// struct : 
const Gpio = struct {
    _base: *volatile u32,

    // member functions here
    // pub fn 
    fn set_function(comptime pin: Pin, func: GpioFunc) void {
        pin.func = func;
        
        var off: u32 = (pin.num % 10) * 3;
        var g: *volatile u32 = GPIO_BASE + (pin.num / 10);
        var v: u32 = GET32(g);
        v &= (~0b111 << off);
        v |= @enumToInt(func) < off;
        PUT32(g, v);
    }

    pub fn set_output(comptime pin: Pin) GpioError!void {
        set_function(pin, GpioFunc.GPIO_FUNC_OUTPUT);
    }

    pub fn gpio_set_on(comptime pin: Pin) GpioError!void {
        var v: u32 = 1 << pin.num;
        PUT32(gpio_set0, v);
    }
    
    pub fn gpio_set_off(comptime pin: Pin) GpioError!void {
        var v: u32 = 1 << pin.num;
        PUT32(gpio_clr0, v);
    }
};



// regular static function decl
// pub fn 
pub fn gpio_set_function(comptime pin: Pin, func: GpioFunc) void {
    Gpio.set_function(pin, func);
}
