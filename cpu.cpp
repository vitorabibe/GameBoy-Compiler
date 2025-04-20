// Current idea: Have this c++ code run the entire logic for the gameboy compiler, 
// having a locally hosted website translate the binary code into screen information.

// TODO: Implement memory reading
// TODO: Implement memory writing
// TODO: Add comments
// TODO: Finish cpu instructions set


#include <cstdint>

class CPU {
private:
    uint8_t a;
    uint8_t f;
    uint8_t b, c;
    uint8_t d, e;
    uint8_t h, l;
    uint16_t pc;
    uint16_t sp;

    bool get_flag_z() { return (f & 0b10000000) != 0; }
    bool get_flag_n() { return (f & 0b01000000) != 0; }
    bool get_flag_h() { return (f & 0b00100000) != 0; }
    bool get_flag_c() { return (f & 0b00010000) != 0; }

    void set_flag_z(bool set) {
        if (set) f |= 0b10000000;
        else f &= ~0b10000000;
    }
    void set_flag_n(bool set) {
        if (set) f |= 0b01000000;
        else f &= ~0b01000000;
    }
    void set_flag_h(bool set) {
        if (set) f |= 0b00100000;
        else f &= ~0b00100000;
    }
    void set_flag_c(bool set) {
        if (set) f |= 0b00010000;
        else f &= ~0b00010000;
    }

    uint16_t get_bc() { return (b << 8) | c; }
    uint16_t get_de() { return (d << 8) | e; }
    uint16_t get_hl() { return (h << 8) | l; }

    void set_bc(uint16_t value) {
        b = value >> 8;
        c = value & 0xFF;
    }
    void set_de(uint16_t value) {
        d = value >> 8;
        e = value & 0xFF;
    }
    void set_hl(uint16_t value) {
        h = value >> 8;
        l = value & 0xFF;
    }

public:
    CPU() {
        a = 0;
        f = 0;
        b = c = 0;
        d = e = 0;
        h = l = 0;
        pc = 0;
        sp = 0;
    }

    void adc_a_r8(uint8_t r8) {
        bool old_carry = get_flag_c();
        uint16_t result = a + r8 + (old_carry ? 1 : 0);
        
        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (r8 & 0xF) + old_carry) > 0xF);
        set_flag_c(result > 0xFF);
        
        a = result & 0xFF;
    }

    void adc_a_hl() {
        uint16_t HL = get_hl();
        uint8_t byte_at_HL = read_byte(HL);
        bool old_carry = get_flag_c();
        uint16_t result = a + byte_at_HL + (old_carry ? 1 : 0);

        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (byte_at_HL & 0xF) + old_carry) > 0xF);
        set_flag_c(result > 0xFF);

        a = result & 0xFF;
    }

    void adc_a_n8(uint8_t n8) {
        bool old_carry = get_flag_c();
        uint16_t result = a + n8 + (old_carry ? 1 : 0);

        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (n8 & 0xF) + old_carry) > 0xF);
        set_flag_c(result > 0xFF);

        a = result & 0xFF;
    }

    void add_a_r8(uint8_t r8) {
        uint16_t result = a + r8;

        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (r8 & 0xF)) > 0xF);
        set_flag_c(result > 0xFF);
        
        a = result & 0xFF;
    }

    void add_a_hl() {
        uint16_t HL = get_hl();
        uint8_t byte_at_HL = read_byte(HL);
        uint16_t result = a + byte_at_HL;

        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (byte_at_HL & 0xF)) > 0xF);
        set_flag_c(result > 0xFF);

        a = result & 0xFF;
    }

    void add_a_n8(uint8_t n8) {
        bool old_carry = get_flag_c();
        uint16_t result = a + n8;

        set_flag_z((result & 0xFF) == 0);
        set_flag_n(false);
        set_flag_h(((a & 0xF) + (n8 & 0xF)) > 0xF);
        set_flag_c(result > 0xFF);

        a = result & 0xFF;
    }

    void add_hl_r16(uint16_t r16) {
        uint16_t HL = get_hl();
        uint32_t result = HL + r16;

        set_flag_n(false);
        set_flag_h(((HL & 0x0FFF) + (r16 & 0x0FFF)) > 0x0FFF);
        set_flag_c(result > 0xFFFF);

        result &= 0xFFFF;
        set_hl(result);
    }

    void add_hl_sp(){
        uint16_t HL = get_hl();
        uint32_t result = HL + sp;

        set_flag_n(false);
        set_flag_h(((HL & 0x0FFF) + (sp & 0x0FFF)) > 0x0FFF);
        set_flag_c(result > 0xFFFF);

        result &= 0xFFFF;
        set_hl(result);
    }

    void add_sp_e8(int8_t e8){
        uint16_t old_sp = sp;
        sp += e8;
        
        set_flag_z(false);
        set_flag_n(false);
        set_flag_h(((old_sp & 0xF) + (e8 & 0xF)) > 0xF);
        set_flag_c(((old_sp & 0xFF) + (e8 & 0xFF)) > 0xFF);
    }

    void and_a_r8(uint8_t r8){
        a &= r8;

        set_flag_z(a == 0);
        set_flag_n(false);
        set_flag_h(true);
        set_flag_c(false);
    }

    void and_a_hl(){
        uint16_t HL = get_hl();
        a &= HL;

        set_flag_z(a == 0);
        set_flag_n(false);
        set_flag_h(true);
        set_flag_c(false);
    }

    void and_a_n8(uint8_t n8){
        a &= n8;

        set_flag_z(a == 0);
        set_flag_n(false);
        set_flag_h(true);
        set_flag_c(false);
    }

    

    uint8_t read_byte(uint16_t address) {
    }

    void write_byte(uint16_t address, uint8_t value) {
    }
};