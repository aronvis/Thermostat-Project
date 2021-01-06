// Function declarion encoder.c
void encoder_init(void);
void set_encoder_state(void);
unsigned char get_low(void);
unsigned char get_high(void);
void set_low(unsigned char temperature);
void set_high(unsigned char temperature);
void init_low(unsigned char temperature);
void init_high(unsigned char temperature);