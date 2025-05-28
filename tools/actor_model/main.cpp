#include "actors.h"
#include "events.h"

#include <library/cpp/actors/core/actor_system.h>
#include <library/cpp/actors/core/actor_id.h>
#include <library/cpp/actors/core/actor_system_setup.h>
#include <library/cpp/actors/util/should_continue.h>

int main() {
    NActors::TActorSystemSetup setup;
    setup.NodeCount = 1;
    setup.ExecutorsCount = 2;
    setup.Metrics = nullptr;

    NActors::TActorSystem system(setup);
    system.Start();

    auto writeActor = system.Register(CreateWriteActor().Release());
    auto readActor = CreateReadActor(std::cin, writeActor);
    system.Register(readActor.Release());

    while (*GetProgramShouldContinue()) {
        Sleep(TDuration::MilliSeconds(10));
    }

    system.Stop();
    return 0;
}
