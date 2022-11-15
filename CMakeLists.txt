cmake_minimum_required(VERSION 3.2)

project(fsEye)

add_executable(fsEye source/main.cpp include/text_processing/tokenizer.h include/localization.h
               include include/stemming/common_lang_constants.h include/stemming/danish_stem.h
               include/stemming/dutch_stem.h include/stemming/english_stem.h
               include/stemming/finnish_stem.h include/stemming/french_stem.h
               include/stemming/german_stem.h include/stemming/italian_stem.h
               include/stemming/norwegian_stem.h include/stemming/portuguese_stem.h
               include/stemming/russian_stem.h include/stemming/spanish_stem.h
               include/stemming/stemming.h include/stemming/swedish_stem.h)
