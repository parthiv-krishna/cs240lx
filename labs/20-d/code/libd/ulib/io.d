module ulib.io;

import ulib.string : itoa;
import ulib.math : min, max;
import ulib.trait : isInt, Unqual;

import sys = ulib.sys;

struct File {
public:
    this(void function(ubyte) putc) {
        this.putc = putc;
    }

    void write(Args...)(Args args) {
        foreach (arg; args) {
            writeElem(arg);
        }
    }

    void flush() {
        // if you implement a buffered writer
    }

private:
    // call this function to write a byte
    void function(ubyte) putc;

    void writeElem(string s) {
        foreach (char ch; s) {
            putc(ch);
        }
    }

    void writeElem(T)(T* val) {
        char[64] buf = void;
        string s = itoa(cast(int)val, buf, 16);
        writeElem("0x");
        writeElem(s);
    }

    void writeElem(char ch) {
        putc(ch);
    }

    void writeElem(bool b) {
        if (b) {
            writeElem("true");
        } else {
            writeElem("false");
        }
    }

    void writeElem(S)(S value, uint base = 10) if (isInt!S) {
        char[64] buf = void;
        string s = itoa(cast(int)value, buf, 10);
        writeElem(s);
    }
}

void write(Args...)(Args args) {
    sys.stdout.write(args);
    sys.stdout.flush();
}

void writeln(Args...)(Args args) {
    sys.stdout.write(args, '\n');
    sys.stdout.flush();
}
