#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
using namespace std;

struct Node
{
  int process;
  int address;
  Node* next;
};

class LinkedList
{
  public:
    Node* head;
    int size;
    LinkedList();
    ~LinkedList();
    int getSize();
    void add(string input, int size);
    void print(ostream &out);
};

LinkedList::LinkedList()
{
  head = nullptr;
  size = 0;
}

LinkedList::~LinkedList()
{
  Node* current = head;
  while(current != nullptr)
  {
    Node* temp = current;
    current = current->next;
    delete temp;
  }
}

int LinkedList::getSize()
{
  return size;
}

void LinkedList::add(string input, int size1)
{
  stringstream ss(input);
  int temp3;
  string temp2;
  
  ss >> temp3;
  Node* temp = new Node();
  temp->process = temp3;
  temp->next = nullptr;
  
  ss >> temp2;
  if(temp2 == "-1")
    temp->address = -1;
  else
  {
    int decimal = 0;
    if (temp2.find("0x") != string::npos)
      stringstream(temp2) >> hex >> decimal;
    
    temp->address = decimal/size1;
  }

  
  if(head == nullptr)
    head = temp;
  else
  {
    Node* current = head;
    while(current->next != nullptr)
      {
        current = current->next;
      }
    current->next = temp;
  }
  size++;
}

void LinkedList::print(ostream &out)
{
  Node* current = head;
  while(current != nullptr)
    {
      out << current->process << " " << current->address << endl;
      current = current->next;
    }
}