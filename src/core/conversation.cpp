#include "core/conversation.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>


// Default constructof function
Conversation::Conversation(){}


// Delete member function
Conversation::~Conversation(){
    delete[] data_;
}


// Copy Constructor Function
Conversation::Conversation(const Conversation& other){
    data_ = new Message[other.capacity_];
    for (std::size_t i = 0; i < other.size_; i++){
        data_[i] = other.data_[i];
        }
    size_ = other.size_;
    capacity_ = other.capacity_;
}


// Copy Assignment Function
Conversation& Conversation::operator=(const Conversation& other){
    if(this != &other){
        delete[] data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = new Message[capacity_];
        for(std::size_t i = 0; i < size_; i++){
            data_[i] = other.data_[i];
        }
    }
        return *this;
}


// Move Constructor Function
Conversation::Conversation(Conversation&& other) noexcept
: data_(other.data_), size_(other.size_), capacity_(other.capacity_){
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
}


// Move Assignment Function
Conversation& Conversation::operator=(Conversation&& other) noexcept{
    if(this != &other){
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        }
    return *this;
}


// Append Function
void Conversation::append(Message m){
    if (size_ == capacity_) {
        std::size_t new_capacity = capacity_ * 2;            //growth factor of 2
        if(capacity_ == 0){
            new_capacity = 1;
        }
        Message* new_data = new Message[new_capacity];
        for (std::size_t i = 0; i < size_; i++)
            new_data[i] = std::move(data_[i]);   // move, don't copy — avoids duplicating the strings
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
        delete[] new_data;
    }
    data_[size_] = std::move(m);
    ++size_;
}


// returns the size of data_ for the current object
std::size_t    Conversation::size() const noexcept{
    return size_;
}


// returns the message at index i of the conversation
const Message& Conversation::at(std::size_t i) const{
    if(i >= size_){
        throw std::out_of_range("Conversation::at: index out of range");
    }
    return data_[i];
}


// returns the message address at the beginning of the conversation
const Message* Conversation::begin() const noexcept{
    return data_;
}


// returns the message address at the end of the conversation
const Message* Conversation::end()   const noexcept{
    return data_ + size_;
}