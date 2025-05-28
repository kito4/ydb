#include "actors.h"
#include "events.h"
#include <library/cpp/actors/core/actor_bootstrapped.h>
#include <library/cpp/actors/core/hfunc.h>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace NActors;
using namespace std::chrono;

class TReadActor : public TActorBootstrapped<TReadActor> {
    std::istream& Strm;
    TActorId Writer;
    size_t Pending = 0;
    bool Finished = false;

public:
    TReadActor(std::istream& strm, const TActorId& writer)
        : Strm(strm), Writer(writer) {}

    void Bootstrap() {
        Become(&TReadActor::StateFunc);
        Send(SelfId(), new TEvWakeup());
    }

    STRICT_STFUNC(StateFunc,
        hFunc(TEvWakeup, HandleWakeup);
        hFunc(TEvents::TEvDone, HandleDone);
    )

    void HandleWakeup(TEvWakeup::TPtr&) {
        int64_t val;
        if (Strm >> val) {
            ++Pending;
            Register(CreateMaximumPrimeDevisorActor(val, SelfId(), Writer).Release());
            Send(SelfId(), new TEvWakeup()); // continue reading
        } else {
            Finished = true;
            if (Pending == 0) {
                Send(Writer, new TEvents::TEvWriteValueRequest(0)); // in case no input
                Send(Writer, new TEvents::TEvPoisonPill::EventType());
                PassAway();
            }
        }
    }

    void HandleDone(TEvents::TEvDone::TPtr&) {
        if (--Pending == 0 && Finished) {
            Send(Writer, new TEvents::TEvPoisonPill::EventType());
            PassAway();
        }
    }
};

THolder<IActor> CreateReadActor(std::istream& strm, const TActorId& writer) {
    return MakeHolder<TReadActor>(strm, writer);
}

class TMaximumPrimeDevisorActor : public TActorBootstrapped<TMaximumPrimeDevisorActor> {
    int64_t Number;
    int64_t MaxPrime = 0;
    int64_t Divisor = 2;
    TActorId Reader;
    TActorId Writer;

public:
    TMaximumPrimeDevisorActor(int64_t number, const TActorId& reader, const TActorId& writer)
        : Number(number), Reader(reader), Writer(writer) {}

    void Bootstrap() {
        Become(&TMaximumPrimeDevisorActor::StateFunc);
        Send(SelfId(), new TEvWakeup());
    }

    STRICT_STFUNC(StateFunc,
        hFunc(TEvWakeup, HandleWakeup);
    )

    void HandleWakeup(TEvWakeup::TPtr&) {
        auto start = steady_clock::now();
        while (Divisor <= Number && duration_cast<milliseconds>(steady_clock::now() - start).count() < 10) {
            if (Number % Divisor == 0) {
                bool isPrime = true;
                for (int64_t i = 2; i * i <= Divisor; ++i) {
                    if (Divisor % i == 0) {
                        isPrime = false;
                        break;
                    }
                }
                if (isPrime) {
                    MaxPrime = Divisor;
                }
            }
            ++Divisor;
        }

        if (Divisor > Number) {
            Send(Writer, new TEvents::TEvWriteValueRequest(MaxPrime));
            Send(Reader, new TEvents::TEvDone());
            PassAway();
        } else {
            Send(SelfId(), new TEvWakeup());
        }
    }
};

THolder<IActor> CreateMaximumPrimeDevisorActor(int64_t number, const TActorId& reader, const TActorId& writer) {
    return MakeHolder<TMaximumPrimeDevisorActor>(number, reader, writer);
}

class TWriteActor : public TActor<TWriteActor> {
    int64_t Sum = 0;

public:
    TWriteActor() : TActor(&TWriteActor::StateFunc) {}

    STRICT_STFUNC(StateFunc,
        hFunc(TEvents::TEvWriteValueRequest, HandleWrite);
        cFunc(TEvents::TEvPoisonPill::EventType, HandlePoison);
    )

    void HandleWrite(TEvents::TEvWriteValueRequest::TPtr& ev) {
        Sum += ev->Get()->Value;
    }

    void HandlePoison() {
        std::cout << Sum << std::endl;
        *GetProgramShouldContinue() = false;
        PassAway();
    }
};

THolder<IActor> CreateWriteActor() {
    return MakeHolder<TWriteActor>();
}
