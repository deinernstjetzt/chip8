#include "app.h"

int main(int argc, char** argv) {
    C8App* app = c8_app_new();
    int res = g_application_run(G_APPLICATION(app), argc, argv);

    return res;
}
