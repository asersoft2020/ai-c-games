#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <time.h>

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_timeout(gpointer user_data);

static int bird_y = 300;
static int bird_velocity = 0;
static const int gravity = 1;
static const int jump_strength = -15;
static const int pipe_width = 80;
static const int pipe_gap = 200;
static int pipe_x = 800;
static int pipe_height = 300;
static int score = 0;

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    srand(time(NULL));

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Flappy Bird");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press_event), NULL);

    g_timeout_add(16, on_timeout, drawing_area); // Approximately 60 FPS

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    // Draw sky
    cairo_set_source_rgb(cr, 0.5, 0.8, 1.0); // Sky blue background
    cairo_paint(cr);

    // Draw bird
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); // Yellow bird
    cairo_arc(cr, 400, bird_y, 20, 0, 2 * G_PI);
    cairo_fill(cr);

    // Draw pipes
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green pipes
    cairo_rectangle(cr, pipe_x, 0, pipe_width, pipe_height);
    cairo_rectangle(cr, pipe_x, pipe_height + pipe_gap, pipe_width, 600 - pipe_height - pipe_gap);
    cairo_fill(cr);

    // Draw score
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black score
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 40);
    cairo_move_to(cr, 10, 50);
    cairo_show_text(cr, g_strdup_printf("Score: %d", score));

    return FALSE;
}

static gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_space) {
        bird_velocity = jump_strength;
    }
    return FALSE;
}

static gboolean on_timeout(gpointer user_data) {
    bird_velocity += gravity;
    bird_y += bird_velocity;

    pipe_x -= 5;
    if (pipe_x < -pipe_width) {
        pipe_x = 800;
        pipe_height = rand() % 400;
        score++;
    }

    if (bird_y > 580 || bird_y < 20 || 
        (pipe_x < 420 && pipe_x + pipe_width > 380 && 
        (bird_y < pipe_height || bird_y > pipe_height + pipe_gap))) {
        bird_y = 300;
        bird_velocity = 0;
        pipe_x = 800;
        score = 0;
    }

    gtk_widget_queue_draw(GTK_WIDGET(user_data));
    return TRUE;
}

// Compile with: gcc main.c -o flappybird `pkg-config --cflags --libs gtk+-3.0`
// Run with: ./flappybird
