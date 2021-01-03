#pragma once
//#define _KB_TASK
#ifdef _KB_TASK
#include <memory>
namespace hungbiu 
{
    template<typename R>
    struct task_concept
    {
        void (*move_)(void*, void*);
        void (*destruct_)(void*);
        R (*invoke_)(void*);
    };

    template <
        typename F, 
        bool Small,
        typename R>
    struct task_model;

    template <typename F>
    struct task_model<F, true, std::result_of_t<F()>>
    {
        static void move_(void* this_, void* oth) noexcept
        {
            auto& m = *(static_cast<task_model*>(oth));
            new (this_) task_model(std::move(m));
        }
        static void destruct_(void* this_) noexcept
        {
            static_cast<task_model*>(this_)->~task_model();
        }
        using R = std::result_of_t<F()>;
        static R invoke_(void* this_)
        {
            return static_cast<task_model*>(this_)->func_();
        }
        static constexpr task_concept concept_{ move_, destruct_, invoke_ };

        F func_;
        task_model(F&& f) :
            func_(std::move(f)) {}
        task_model(const task_model&) = delete;
        task_model(task_model&& oth) noexcept :
            func_(std::move(oth.func_)) {}
        ~task_model() {}
    };
    template <typename F>
    struct task_model<F, false, std::result_of_t<F()>>
    {
        static void move_(void* this_, void* oth) noexcept
        {
            auto& m = *(static_cast<task_model*>(oth));
            new (this_) task_model(std::move(m));
        }
        static void destruct_(void* this_) noexcept
        {
            static_cast<task_model*>(this_)->~task_model();
        }
        using R = std::result_of_t<F()>;
        static R invoke_(void* this_)
        {
            auto& f = *(static_cast<task_model*>(this_)->pfunc_);
            return f.operator()();
        }
        static constexpr task_concept concept_{ move_, destruct_, invoke_ };

        std::unique_ptr<F> pfunc_;
        task_model(F&& f)
        {
            pfunc_ = std::make_unique<F>(std::move(f));
        }
        task_model(const task_model&) = delete;
        task_model(task_model&& oth) noexcept :
            pfunc_(std::move(oth.pfunc_)) {}
        ~task_model() {}
    };

    struct task
    {
        const task_concept* pconcept_ = nullptr;
        static constexpr auto SMALL_SZ = sizeof(void*) * 4;
        std::aligned_storage_t<SMALL_SZ> storage_;
        void* this_()
        {
            return static_cast<void*>(&storage_);
        }

        template <
            typename F,
            typename = std::enable_if_t<std::is_same_v<F, task> == false>
        >
            task(F&& f)
        {
            using decay_f = std::decay_t<F>;
            constexpr bool is_small = sizeof(decay_f) <= SMALL_SZ;
            using model_t = task_model<decay_f, is_small>;
            static_assert(sizeof(model_t) <= SMALL_SZ);

            new (this_()) model_t(std::forward<decay_f>(f));
            pconcept_ = &model_t::concept_;

        }
        task(task&& oth) noexcept :
            pconcept_(oth.pconcept_)
        {
            pconcept_->move_(this_(), oth.this_());
        }
        ~task()
        {
            pconcept_->destruct_(this_());
        }
        task& operator=(task&& rhs) noexcept
        {
            if (this != &rhs)
            {
                pconcept_->destruct_(this_()); // rhs may contains a different model_t!
                pconcept_ = rhs.pconcept_;
                pconcept_->move_(this_(), rhs.this_());
            }
            return *this;
        }
        void operator()()
        {
            pconcept_->invoke_(this_());
        }
    };
};
#endif