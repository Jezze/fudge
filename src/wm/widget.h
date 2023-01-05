#define WIDGET_COMMAND_NONE                     1
#define WIDGET_COMMAND_COMMENT                  2
#define WIDGET_COMMAND_DELETE                   3
#define WIDGET_COMMAND_INSERT                   4
#define WIDGET_COMMAND_UPDATE                   5
#define WIDGET_TYPE_BUTTON                      1
#define WIDGET_TYPE_FILL                        2
#define WIDGET_TYPE_GRID                        3
#define WIDGET_TYPE_IMAGE                       4
#define WIDGET_TYPE_CHOICE                      5
#define WIDGET_TYPE_LAYOUT                      6
#define WIDGET_TYPE_SELECT                      7
#define WIDGET_TYPE_TEXT                        8
#define WIDGET_TYPE_TEXTBOX                     9
#define WIDGET_TYPE_TEXTBUTTON                  10
#define WIDGET_TYPE_WINDOW                      11
#define WIDGET_STATE_NORMAL                     0
#define WIDGET_STATE_HOVER                      1
#define WIDGET_STATE_HOVEROFF                   2
#define WIDGET_STATE_FOCUS                      3
#define WIDGET_STATE_FOCUSOFF                   4
#define WIDGET_STATE_DESTROYED                  5
#define WIDGET_ATTR_COLOR                       1
#define WIDGET_ATTR_COLUMNS                     2
#define WIDGET_ATTR_CONTENT                     3
#define WIDGET_ATTR_HALIGN                      4
#define WIDGET_ATTR_ID                          5
#define WIDGET_ATTR_IN                          6
#define WIDGET_ATTR_LABEL                       7
#define WIDGET_ATTR_MODE                        8
#define WIDGET_ATTR_PADDING                     9
#define WIDGET_ATTR_PLACEMENT                   10
#define WIDGET_ATTR_SOURCE                      11
#define WIDGET_ATTR_TITLE                       12
#define WIDGET_ATTR_TYPE                        13
#define WIDGET_ATTR_WEIGHT                      14
#define WIDGET_ATTR_VALIGN                      15
#define WIDGET_ATTR_WRAP                        16
#define GRID_PLACEMENT_NORMAL                   0
#define GRID_PLACEMENT_STRETCHED                1
#define IMAGE_TYPE_FUDGEMOUSE                   1
#define IMAGE_TYPE_PCX                          2
#define LAYOUT_TYPE_FLOAT                       0
#define LAYOUT_TYPE_HORIZONTAL                  1
#define LAYOUT_TYPE_MAXIMIZE                    2
#define LAYOUT_TYPE_VERTICAL                    3
#define LAYOUT_PLACEMENT_NORMAL                 0
#define LAYOUT_PLACEMENT_STRETCHED              1
#define TEXTBOX_MODE_NORMAL                     0
#define TEXTBOX_MODE_READONLY                   1

struct widget_position
{

    int x;
    int y;

};

struct widget_size
{

    unsigned int w;
    unsigned int h;

};

struct widget_button
{

    unsigned int label;

};

struct widget_choice
{

    unsigned int label;

};

struct widget_layout
{

    unsigned int type;
    unsigned int placement;
    unsigned int padding;

};

struct widget_fill
{

    unsigned int color;

};

struct widget_grid
{

    unsigned int columns;
    unsigned int placement;
    unsigned int padding;

};

struct widget_image
{

    unsigned int type;
    unsigned int source;

};

struct widget_select
{

    unsigned int label;

};

struct widget_text
{

    unsigned int content;
    unsigned int halign;
    unsigned int valign;
    unsigned int mode;
    unsigned int weight;
    unsigned int wrap;
    unsigned int firstrowx;
    struct text_info textinfo;

};

struct widget_textbox
{

    unsigned int mode;
    int scroll;

};

struct widget_textbutton
{

    unsigned int label;

};

struct widget_window
{

    unsigned int title;

};

struct widget_cache
{

    unsigned int exist;
    union
    {

        struct
        {

            unsigned int num;
            unsigned int start;
            unsigned int length;
            struct text_rowinfo info;

        } textrow;
        struct
        {
            struct widget_size size;
        } image;

    } payload;

};

struct widget
{

    unsigned int source;
    unsigned int type;
    unsigned int state;
    unsigned int id;
    unsigned int in;
    void *data;
    struct widget_position position;
    struct widget_size size;
    struct widget_cache cache;

};

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value);
void widget_unsetattributes(struct widget *widget);
unsigned int widget_getattribute(char *value);
unsigned int widget_getcommand(char *value);
unsigned int widget_gettype(char *value);
unsigned int widget_setstate(struct widget *widget, unsigned int state);
unsigned int widget_intersectsx(struct widget *widget, int x);
unsigned int widget_intersectsy(struct widget *widget, int y);
unsigned int widget_intersects(struct widget *widget, int x, int y);
unsigned int widget_isdragable(struct widget *widget);
unsigned int widget_isinteractive(struct widget *widget);
void widget_initposition(struct widget_position *position, int x, int y);
void widget_initsize(struct widget_size *size, int w, int h);
void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data);