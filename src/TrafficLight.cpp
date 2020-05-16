#include <iostream>
#include <random>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mut);
    _cv.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mut);
    _queue.push_back(std::move(msg));
    _cv.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    auto msg = _messages.receive();
    while (true) {
        msg = _messages.receive();
        if (msg == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    while (true) {
        srand((unsigned) time(0));
        if (_currentPhase == red) {
            _currentPhase = green;
        } else {
            _currentPhase = red;
	    }
        _messages.send(std::move(_currentPhase));
        int randNum = rand() % (6 + 1 - 4) + 4;
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsed = (std::chrono::steady_clock::duration) 0;
        std::chrono::steady_clock::duration timer = std::chrono::seconds(randNum);
        while (elapsed.count() < timer.count()) {
            elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}