#include "common.h"

int main(int argc, char **argv)
{
    auto engine = GameEngine();
    return engine.run(argc, argv);
}