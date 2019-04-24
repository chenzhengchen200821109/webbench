#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

namespace libco++
{
    class EventLoop {
        public:
            typedef std::function<void()> Functor;

            EventLoop();

            //Build an EventLoop using an existing event_base object,
            // so we can embed an EventLoop object into the old applications based on libevent
            // NOTE: Be careful to deal with the destructing work of event_base_ and watcher_ objects.
            explicit EventLoop(struct event_base *base);
            ~EventLoop();

            // Run the IO Event driving loop forever
            // It MUST be called in the IO Event thread
            void Run();

            // Stop the event loop
            void Stop();

            // Reinitialize some data fields after a fork
            void AfterFork();

            void RunAfter(double delay_ms, const Functor& f);

            // RunEvery executes Functor f every period interval time.
            void RunEvery(Duration interval, const Functor& f);

        private:

    };

} // namespace libco++





#endif 
