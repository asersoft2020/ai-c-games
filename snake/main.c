#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 20
#define WIDTH 800
#define HEIGHT 600

typedef struct {
    int x, y;
} Point;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_timeout(gpointer user_data);

static Point snake[100];
static int snake_length = 5;
static Point food;
static int direction = GDK_KEY_Right;
static gboolean game_over = FALSE;

void place_food() {
    food.x = rand() % (WIDTH / GRID_SIZE);
    food.y = rand() % (HEIGHT / GRID_SIZE);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    srand(time(NULL));

    for (int i = 0; i < snake_length; i++) {
        snake[i].x = snake_length - i - 1;
        snake[i].y = 0;
    }

    place_food();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Snake Game");
    gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press_event), NULL);

    g_timeout_add(100, on_timeout, drawing_area); // Game speed

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    // Draw background
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); // Gray background
    cairo_paint(cr);

    // Draw snake
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green snake
    for (int i = 0; i < snake_length; i++) {
        cairo_rectangle(cr, snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
        cairo_fill(cr);
    }

    // Draw food
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // Red food
    cairo_rectangle(cr, food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE, GRID_SIZE);
    cairo_fill(cr);

    // Draw game over text
    if (game_over) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, WIDTH / 2 - 100, HEIGHT / 2);
        cairo_show_text(cr, "Game Over");
    }

    // Draw snake length text
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, g_strdup_printf("Length: %d", snake_length));

    return FALSE;
}

static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_Up && direction != GDK_KEY_Down) {
        direction = GDK_KEY_Up;
    } else if (event->keyval == GDK_KEY_Down && direction != GDK_KEY_Up) {
        direction = GDK_KEY_Down;
    } else if (event->keyval == GDK_KEY_Left && direction != GDK_KEY_Right) {
        direction = GDK_KEY_Left;
    } else if (event->keyval == GDK_KEY_Right && direction != GDK_KEY_Left) {
        direction = GDK_KEY_Right;
    }
    return FALSE;
}

static gboolean on_timeout(gpointer user_data) {
    if (game_over) {
        return FALSE;
    }

    // Move snake
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    if (direction == GDK_KEY_Up) {
        snake[0].y--;
    } else if (direction == GDK_KEY_Down) {
        snake[0].y++;
    } else if (direction == GDK_KEY_Left) {
        snake[0].x--;
    } else if (direction == GDK_KEY_Right) {
        snake[0].x++;
    }

    // Check for collision with food
    if (snake[0].x == food.x && snake[0].y == food.y) {
        snake_length++;
        place_food();
    }

    // Check for collision with walls
    if (snake[0].x < 0 || snake[0].x >= WIDTH / GRID_SIZE || snake[0].y < 0 || snake[0].y >= HEIGHT / GRID_SIZE) {
        game_over = TRUE;
    }

    // Check for collision with itself
    for (int i = 1; i < snake_length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            game_over = TRUE;
        }
    }

    gtk_widget_queue_draw(GTK_WIDGET(user_data));
    return TRUE;
}

//execute: gcc -o snake snake.c `pkg-config --cflags --libs gtk+-3.0`
//run: ./snake
//end