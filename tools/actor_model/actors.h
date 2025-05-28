#pragma once

#include <library/cpp/actors/core/actor.h>
#include <istream>
#include <memory>

THolder<NActors::IActor> CreateReadActor(std::istream& strm, const NActors::TActorId& writer);
THolder<NActors::IActor> CreateMaximumPrimeDevisorActor(int64_t number, const NActors::TActorId& reader, const NActors::TActorId& writer);
THolder<NActors::IActor> CreateWriteActor();
