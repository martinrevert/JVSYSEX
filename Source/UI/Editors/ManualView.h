#pragma once

namespace jv1010 {

class ManualView {
public:
    void render();

private:
    int selectedSection_{ 0 };
    bool spanish_{ true };
};

} // namespace jv1010
