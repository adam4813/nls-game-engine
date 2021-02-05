/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Constants
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const uint WM_KEYFIRST                   = 0x0100;
const uint WM_KEYDOWN                    = 0x0100;
const uint WM_KEYUP                      = 0x0101;
const uint WM_CLOSE                      = 0x0010;


/*
 * Virtual Keys, Standard Set
 */
const uint VK_LBUTTON      = 0x01;
const uint VK_RBUTTON      = 0x02;
const uint VK_CANCEL       = 0x03;
const uint VK_MBUTTON      = 0x04;   /* NOT contiguous with L & RBUTTON */
const uint VK_XBUTTON1     = 0x05;   /* NOT contiguous with L & RBUTTON */
const uint VK_XBUTTON2     = 0x06;   /* NOT contiguous with L & RBUTTON */

const uint VK_BACK         = 0x08;
const uint VK_TAB          = 0x09;

const uint VK_CLEAR        = 0x0C;
const uint VK_RETURN       = 0x0D;
const uint VK_SHIFT        = 0x10;
const uint VK_CONTROL      = 0x11;
const uint VK_MENU         = 0x12;
const uint VK_PAUSE        = 0x13;
const uint VK_CAPITAL      = 0x14;

const uint VK_ESCAPE       = 0x1B;

const uint VK_SPACE        = 0x20;
const uint VK_PRIOR        = 0x21;
const uint VK_NEXT         = 0x22;
const uint VK_END          = 0x23;
const uint VK_HOME         = 0x24;

const uint VK_LEFT         = 0x25;
const uint VK_UP           = 0x26;
const uint VK_RIGHT        = 0x27;
const uint VK_DOWN         = 0x28;

const uint VK_SELECT       = 0x29;
const uint VK_PRINT        = 0x2A;
const uint VK_EXECUTE      = 0x2B;
const uint VK_SNAPSHOT     = 0x2C;
const uint VK_INSERT       = 0x2D;
const uint VK_DELETE       = 0x2E;
const uint VK_HELP         = 0x2F;

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */
const uint VK_0            = 0x30;
const uint VK_1            = 0x31;
const uint VK_2            = 0x32;
const uint VK_3            = 0x33;
const uint VK_4            = 0x34;
const uint VK_5            = 0x35;
const uint VK_6            = 0x36;
const uint VK_7            = 0x37;
const uint VK_8            = 0x38;
const uint VK_9            = 0x39;

const uint VK_LWIN         = 0x5B;
const uint VK_RWIN         = 0x5C;
const uint VK_APPS         = 0x5D;

const uint VK_SLEEP        = 0x5F;

const uint VK_NUMPAD0      = 0x60;
const uint VK_NUMPAD1      = 0x61;
const uint VK_NUMPAD2      = 0x62;
const uint VK_NUMPAD3      = 0x63;
const uint VK_NUMPAD4      = 0x64;
const uint VK_NUMPAD5      = 0x65;
const uint VK_NUMPAD6      = 0x66;
const uint VK_NUMPAD7      = 0x67;
const uint VK_NUMPAD8      = 0x68;
const uint VK_NUMPAD9      = 0x69;

const uint VK_MULTIPLY     = 0x6A;
const uint VK_ADD          = 0x6B;
const uint VK_SEPARATOR    = 0x6C;
const uint VK_SUBTRACT     = 0x6D;
const uint VK_DECIMAL      = 0x6E;
const uint VK_DIVIDE       = 0x6F;

const uint VK_F1           = 0x70;
const uint VK_F2           = 0x71;
const uint VK_F3           = 0x72;
const uint VK_F4           = 0x73;
const uint VK_F5           = 0x74;
const uint VK_F6           = 0x75;
const uint VK_F7           = 0x76;
const uint VK_F8           = 0x77;
const uint VK_F9           = 0x78;
const uint VK_F10          = 0x79;
const uint VK_F11          = 0x7A;
const uint VK_F12          = 0x7B;
const uint VK_F13          = 0x7C;
const uint VK_F14          = 0x7D;
const uint VK_F15          = 0x7E;
const uint VK_F16          = 0x7F;
const uint VK_F17          = 0x80;
const uint VK_F18          = 0x81;
const uint VK_F19          = 0x82;
const uint VK_F20          = 0x83;
const uint VK_F21          = 0x84;
const uint VK_F22          = 0x85;
const uint VK_F23          = 0x86;
const uint VK_F24          = 0x87;

const uint VK_NUMLOCK      = 0x90;
const uint VK_SCROLL       = 0x91;

/*
 * Mouse Buttons
 */
 
const uint WM_LBUTTONDOWN = 0x0201;
const uint WM_LBUTTONUP = 0x0202;
const uint WM_MBUTTONDOWN = 0x0207;
const uint WM_MBUTTONUP = 0x0208;
const uint WM_MOUSEHWHEEL = 0x020E;
const uint WM_MOUSEMOVE = 0x0200;
const uint WM_MOUSEWHEEL = 0x020A;
const uint WM_RBUTTONDOWN = 0x0204;
const uint WM_RBUTTONUP = 0x0205;
const uint WM_XBUTTONDOWN = 0x020B;
const uint WM_XBUTTONUP = 0x020C;