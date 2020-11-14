cmake_minimum_required(VERSION 3.17)
project(oppgave_7_server C)

set(CMAKE_C_STANDARD 90)

add_executable(oppgave_7_server main.c mop.c threadmanager.c simplelogger.c)