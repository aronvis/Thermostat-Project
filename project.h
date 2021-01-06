// Function declarion project.c
// -------- LCD Functions --------
void print_low();
void print_high();
void print_room_temp(unsigned short * curr_temp);
void convert_temp(unsigned char * recorded_temp, unsigned short * curr_temp);
unsigned char changed_temp(unsigned short * prev_temp, unsigned short * curr_temp);
void update_prev_temp(unsigned short * prev_temp, unsigned short * curr_temp);

// -------- Button Functions --------
unsigned char change_low();
unsigned char change_high();

// -------- LED Functions ----------
void manage_temp(unsigned short * curr_temp);
unsigned char cooling();
unsigned char heating();
void cool_on();
void cool_off();
void heat_on();
void heat_off();

// Shared global variables for encoder program
volatile unsigned char encoder_changed = 0; 
volatile unsigned char new_state, old_state;
volatile unsigned char a, b;
volatile unsigned char inputBits;
volatile unsigned char temperature = 0; 
volatile unsigned char low_state = 0;
volatile unsigned char low = 68;
volatile unsigned char high = 72;
