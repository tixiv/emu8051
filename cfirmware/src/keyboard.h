
// public read only
extern char current_key;

// public read / write: 'update_keyboard()' puts new  keystrokes, user takes them
extern char key_buffer;

void update_keyboard(void);
