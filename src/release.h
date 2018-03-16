#ifndef RELEASE_H
#define RELEASE_H

#include <memory>

template <typename F>
class Release
{
public:
    Release(F f) : f_{std::move(f)} {}

    ~Release() {
        f_();
    }

private:
    F f_;
};

template <typename F>
auto make_release(F f) {
    return Release<F>(std::move(f));
}



#endif // RELEASE_H
