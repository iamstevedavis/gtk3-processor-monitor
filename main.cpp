#include </usr/include/gtk-3.0/gtk/gtk.h>
#include <fstream>
#include <glib.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

using namespace std;
#define FREQUENCY 5000
/**
 * TODO: Fix this global. Need to create a struct that holds
 * all the labels and a pointer to the last label address.
 * I can pass that struct to update_text and the loop can go until
 * we reach that last address.
 * https://stackoverflow.com/questions/41687921/looping-through-array-using-pointers
 */
int processorCount = 0;

// TODO: Add thermal data from here /sys/class/thermal

int getProcessorCount() {
  ifstream infile("/proc/cpuinfo");
  int count = 0;

  string line;
  while (getline(infile, line)) {
    istringstream iss(line);
    if (line.find("siblings") != string::npos) {
      count = (stof(
          line.substr(line.find(":") + 2, (line.length() - line.find(":")))));
      break;
    }
  }
  return count;
}

float *getProcessorSpeeds() {
  ifstream infile("/proc/cpuinfo");

  float *speeds = new float[processorCount];
  int count = 0;
  string line;
  while (getline(infile, line)) {
    istringstream iss(line);
    if (line.find("cpu MHz") != string::npos) {
      speeds[count] =
          (stof(line.substr(line.find(":") + 2,
                            (line.find(".") - 2 - line.find(":")))) /
           1000);
      count++;
    }
  }

  return speeds;
}

static gboolean updateText(GtkWidget **labels) {
  float *processorSpeeds = getProcessorSpeeds();

  for (int i = 0; i < processorCount; i++) {

    gtk_label_set_text(
        GTK_LABEL(labels[i]),
        ("cpu GHz      :  " + to_string(processorSpeeds[i])).c_str());
  }

  return TRUE;
}

static void infoModal(GtkWidget *wid, GtkWidget *win) {
  GtkWidget *dialog = NULL;

  dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                  "Information Here!");
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
  processorCount = getProcessorCount();
  GtkWidget *button = NULL;
  GtkWidget **labels;
  GtkWidget *win = NULL;
  GtkWidget *vbox = NULL;
  labels = g_new(GtkWidget *, processorCount);

  /* Initialize GTK+ */
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc)gtk_false, NULL);
  gtk_init(&argc, &argv);
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  /* Create the main window */
  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(win), 80);
  gtk_window_set_title(GTK_WINDOW(win), "Monitor");
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
  gtk_widget_realize(win);
  g_signal_connect(win, "destroy", gtk_main_quit, NULL);

  /* Create a vertical box with buttons */
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add(GTK_CONTAINER(win), vbox);

  for (int i = 0; i < processorCount; i++) {
    labels[i] = gtk_label_new("cpu GHz      :  -- ");
    gtk_box_pack_start(GTK_BOX(vbox), labels[i], TRUE, TRUE, 0);
  }

  // TODO: Remove timeout when window is closed
  g_timeout_add(FREQUENCY, (GSourceFunc)updateText, labels);
  updateText(labels);

  button = gtk_button_new_with_label("Info");
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(infoModal),
                   (gpointer)win);
  gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label("Close");
  g_signal_connect(button, "clicked", gtk_main_quit, NULL);
  gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

  /* Enter the main loop */
  gtk_widget_show_all(win);
  gtk_main();
  return 0;
}
