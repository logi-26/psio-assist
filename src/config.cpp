#include "config.h"

void Config::load() {
    lastDirectory = settings.value("lastDirectory", "").toString();
}

void Config::save() {
    settings.setValue("lastDirectory", lastDirectory);
    settings.sync();
} 