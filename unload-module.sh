#!/bin/sh

# Shortcut for unloading the networkmanagement kded module manually

qdbus org.kde.kded /kded unloadModule networkmanagement
