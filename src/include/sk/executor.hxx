/*
 * Copyright (c) 2019, 2020, 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SK_EXECUTOR_HXX_INCLUDED
#define SK_EXECUTOR_HXX_INCLUDED

#include <functional>
#include <future>
#include <memory>
#include <deque>

namespace sk {

    /*************************************************************************
     *
     * executor: dispatch tasks.
     *
     */
    struct executor {
        virtual auto post(std::function<void()> &&) -> void = 0;
    };

    /*************************************************************************
     *
     * mt_executor: thread-pool executor.
     *
     * The mt_executor runs forever until its stop() method is called.
     *
     */

    struct mt_executor final : executor {
        mt_executor() = default;
        mt_executor(mt_executor const &) = delete;
        mt_executor(mt_executor &&) = delete;
        auto operator= (mt_executor const &) -> mt_executor & = delete;
        auto operator= (mt_executor &&) -> mt_executor & = delete;
        ~mt_executor();

        using work_type = std::function<void()>;

        // Post work to the queue
        auto post(work_type &&work) -> void final;

        // Run the workq until the exit flag is set.
        auto run() -> void;

        // Start n threads which all run the workq.
        auto start_threads(unsigned int nthreads =
                               std::thread::hardware_concurrency()) -> void;

        // Shut down the work queue and wait for all executing
        // threads to exit.
        auto stop() -> void;

    private:
        std::condition_variable _cv;
        std::mutex _mtx;
        std::deque<work_type> _work;
        std::vector<std::thread> _threads;
        bool _stop = false;
    };

    inline mt_executor::~mt_executor() {
        if (!_threads.empty())
            stop();
    }

    inline void mt_executor::post(work_type &&work)
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _work.push_back(std::move(work));
        _cv.notify_one();
    }

    inline auto mt_executor::run() -> void
    {
        for (;;) {
            std::unique_lock<std::mutex> lock(_mtx);
            _cv.wait(lock, [&] { return !_work.empty() || _stop; });

            if (_stop)
                return;

            auto work = _work.front();
            _work.pop_front();
            lock.unlock();

            work();
        }
    }

    inline auto mt_executor::stop() -> void
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _stop = true;
        _cv.notify_all();
        lock.unlock();

        while (!_threads.empty()) {
            _threads.back().join();
            _threads.pop_back();
        }

        _threads.clear();
    }

    inline auto mt_executor::start_threads(unsigned int nthreads) -> void
    {
        if (nthreads == 0)
            nthreads = 1;

        while (nthreads-- > 0u)
            _threads.emplace_back(&mt_executor::run, this);
    }

    /*************************************************************************
     *
     * st_executor: single-threaded executor.
     *
     * The st_executor runs until no more work is available, then it returns.
     * At least one work item should be post()ed before calling run().
     */

    struct st_executor final : executor {
        using work_type = std::function<void()>;

        // Post work to the queue
        auto post(work_type &&work) -> void final;

        // Run the executor until no work is available.
        auto run() -> void;

    private:
        std::deque<work_type> _work;
    };

    inline void st_executor::post(work_type &&work)
    {
        _work.push_back(std::move(work));
    }

    inline auto st_executor::run() -> void
    {
        while (!_work.empty()) {
            auto work = _work.front();
            _work.pop_front();
            work();
        }
    }

} // namespace sk

#endif // SK_EXECUTOR_HXX_INCLUDED
