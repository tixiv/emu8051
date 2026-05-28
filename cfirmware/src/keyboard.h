
// public read only
extern char current_key;

// public read / write: 'update_keyboard()' puts new  keystrokes, user takes them
extern char key_buffer;

void update_keyboard(void);

#define KEY_ENTER 1
#define KEY_ESC 2
#define KEY_MEM 3
#define KEY_MENU 4
