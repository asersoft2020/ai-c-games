#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 30
#define WIDTH 300
#define HEIGHT 600
#define ROWS (HEIGHT / GRID_SIZE)
#define COLS (WIDTH / GRID_SIZE)

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point blocks[4];
    int color;
} Tetromino;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_timeout(gpointer user_data);

static int grid[ROWS][COLS] = {0};
static Tetromino current_tetromino;
static gboolean game_over = FALSE;
static int score = 0;
static int level = 1;
static time_t start_time;
static guint timeout_id;

void init_tetromino() {
    int type = rand() % 7;
    switch (type) {
        case 0: // I
            current_tetromino.blocks[0] = (Point){4, 0};
            current_tetromino.blocks[1] = (Point){5, 0};
            current_tetromino.blocks[2] = (Point){6, 0};
            current_tetromino.blocks[3] = (Point){7, 0};
            break;
        case 1: // O
            current_tetromino.blocks[0] = (Point){5, 0};
            current_tetromino.blocks[1] = (Point){6, 0};
            current_tetromino.blocks[2] = (Point){5, 1};
            current_tetromino.blocks[3] = (Point){6, 1};
            break;
        case 2: // T
            current_tetromino.blocks[0] = (Point){5, 0};
            current_tetromino.blocks[1] = (Point){4, 1};
            current_tetromino.blocks[2] = (Point){5, 1};
            current_tetromino.blocks[3] = (Point){6, 1};
            break;
        case 3: // S
            current_tetromino.blocks[0] = (Point){5, 0};
            current_tetromino.blocks[1] = (Point){6, 0};
            current_tetromino.blocks[2] = (Point){4, 1};
            current_tetromino.blocks[3] = (Point){5, 1};
            break;
        case 4: // Z
            current_tetromino.blocks[0] = (Point){4, 0};
            current_tetromino.blocks[1] = (Point){5, 0};
            current_tetromino.blocks[2] = (Point){5, 1};
            current_tetromino.blocks[3] = (Point){6, 1};
            break;
        case 5: // J
            current_tetromino.blocks[0] = (Point){4, 0};
            current_tetromino.blocks[1] = (Point){4, 1};
            current_tetromino.blocks[2] = (Point){5, 1};
            current_tetromino.blocks[3] = (Point){6, 1};
            break;
        case 6: // L
            current_tetromino.blocks[0] = (Point){6, 0};
            current_tetromino.blocks[1] = (Point){4, 1};
            current_tetromino.blocks[2] = (Point){5, 1};
            current_tetromino.blocks[3] = (Point){6, 1};
            break;
    }
    current_tetromino.color = rand() % 6 + 1;
}

int check_collision(int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        int new_x = current_tetromino.blocks[i].x + dx;
        int new_y = current_tetromino.blocks[i].y + dy;
        if (new_x < 0 || new_x >= COLS || new_y >= ROWS || (new_y >= 0 && grid[new_y][new_x])) {
            return 1;
        }
    }
    return 0;
}

void place_tetromino() {
    for (int i = 0; i < 4; i++) {
        int x = current_tetromino.blocks[i].x;
        int y = current_tetromino.blocks[i].y;
        if (y >= 0) {
            grid[y][x] = current_tetromino.color;
        }
    }
}

void clear_lines(gpointer user_data) {
    for (int y = 0; y < ROWS; y++) {
        int full = 1;
        for (int x = 0; x < COLS; x++) {
            if (!grid[y][x]) {
                full = 0;
                break;
            }
        }
        if (full) {
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < COLS; x++) {
                    grid[yy][x] = grid[yy - 1][x];
                }
            }
            for (int x = 0; x < COLS; x++) {
                grid[0][x] = 0;
            }
            score += 100;
            if (score % 500 == 0) {
                level++;
                g_source_remove(timeout_id);
                timeout_id = g_timeout_add(500 - (level * 50), on_timeout, user_data); // Increase speed
            }
        }
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    srand(time(NULL));
    init_tetromino();
    start_time = time(NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tetris");
    gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press_event), NULL);

    timeout_id = g_timeout_add(500, on_timeout, drawing_area); // Initial game speed

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    // Draw background
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black background
    cairo_paint(cr);

    // Draw grid
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (grid[y][x]) {
                switch (grid[y][x]) {
                    case 1: cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); break; // Red
                    case 2: cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); break; // Green
                    case 3: cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); break; // Blue
                    case 4: cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); break; // Yellow
                    case 5: cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); break; // Magenta
                    case 6: cairo_set_source_rgb(cr, 0.0, 1.0, 1.0); break; // Cyan
                }
                cairo_rectangle(cr, x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
                cairo_fill(cr);
            }
        }
    }

    // Draw current tetromino
    for (int i = 0; i < 4; i++) {
        int x = current_tetromino.blocks[i].x;
        int y = current_tetromino.blocks[i].y;
        if (y >= 0) {
            switch (current_tetromino.color) {
                case 1: cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); break; // Red
                case 2: cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); break; // Green
                case 3: cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); break; // Blue
                case 4: cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); break; // Yellow
                case 5: cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); break; // Magenta
                case 6: cairo_set_source_rgb(cr, 0.0, 1.0, 1.0); break; // Cyan
            }
            cairo_rectangle(cr, x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
            cairo_fill(cr);
        }
    }

    // Draw game over text
    if (game_over) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, WIDTH / 2 - 100, HEIGHT / 2);
        cairo_show_text(cr, "Game Over");
    }

    // Draw level, time, and score
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, g_strdup_printf("Level: %d", level));
    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, g_strdup_printf("Score: %d", score));
    cairo_move_to(cr, 10, 60);
    cairo_show_text(cr, g_strdup_printf("Time: %ld", time(NULL) - start_time));

    return FALSE;
}

static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (game_over) {
        return FALSE;
    }

    if (event->keyval == GDK_KEY_Left && !check_collision(-1, 0)) {
        for (int i = 0; i < 4; i++) {
            current_tetromino.blocks[i].x--;
        }
    } else if (event->keyval == GDK_KEY_Right && !check_collision(1, 0)) {
        for (int i = 0; i < 4; i++) {
            current_tetromino.blocks[i].x++;
        }
    } else if (event->keyval == GDK_KEY_Down && !check_collision(0, 1)) {
        for (int i = 0; i < 4; i++) {
            current_tetromino.blocks[i].y++;
        }
    } else if (event->keyval == GDK_KEY_Up) {
        // Rotate tetromino (simple 90 degree rotation)
        Point pivot = current_tetromino.blocks[1];
        Tetromino rotated = current_tetromino;
        for (int i = 0; i < 4; i++) {
            int x = current_tetromino.blocks[i].x - pivot.x;
            int y = current_tetromino.blocks[i].y - pivot.y;
            rotated.blocks[i].x = pivot.x - y;
            rotated.blocks[i].y = pivot.y + x;
        }
        if (!check_collision(0, 0)) {
            current_tetromino = rotated;
        }
    }
    gtk_widget_queue_draw(widget);
    return FALSE;
}

static gboolean on_timeout(gpointer user_data) {
    if (game_over) {
        return FALSE;
    }

    if (!check_collision(0, 1)) {
        for (int i = 0; i < 4; i++) {
            current_tetromino.blocks[i].y++;
        }
    } else {
        place_tetromino();
        clear_lines(user_data);
        init_tetromino();
        if (check_collision(0, 0)) {
            game_over = TRUE;
        }
    }

    gtk_widget_queue_draw(GTK_WIDGET(user_data));
    return TRUE;
}