//#pragma once
//#include <vector>
//#include <memory>
//#include <typeinfo>
//
//
//
//class EffectManager {
//public:
//    void Init();
//    void Cleanup();
//    void Update();
//    void Draw(int mainScreen);
//
//    template<typename T>
//    void ToggleEffect()
//    {
//        for (auto& e : effects_) {
//            if (typeid(*e) == typeid(T)) {
//                e->SetEnabled(!e->IsEnabled());
//                return;
//            }
//        }
//    }
//
//    template<typename T>
//    T* Add() {
//        auto effect = std::make_unique<T>();
//        effect->Init();
//        T* ptr = effect.get();
//        effects_.push_back(std::move(effect));
//        return ptr;
//    }
//
//private:
// 
//    int pingPong_[2] = { -1, -1 };
//};