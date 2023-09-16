//
// Created by os on 8/17/22.
//
#include "../h/SystemConsole.h"
sem_t SystemConsole::outputSem = nullptr;
sem_t SystemConsole::extIntSemForOutput = nullptr;
MyBuffer* SystemConsole::inputBuffer = nullptr;
MyBuffer* SystemConsole::outputBuffer = nullptr;

SystemConsole* SystemConsole::console = nullptr;