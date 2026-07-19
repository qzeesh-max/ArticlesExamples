#include <iostream>

using namespace std;

struct SampleStruct
{
 private:
  string Name;


 public:
  SampleStruct(const string& sName): Name(sName) { } 

  void printHello()
  {
    cout << "Hello " << Name << endl; 
  }

  void printBye()
  {
    cout << "Bye " << Name << endl; 
  }

  void printBadCommand()
  {
    cout << "Bad Command!" << endl; 
  }
};


int main()
{
  string Name;

  cout << "Your name:";
  cin >> Name;

  SampleStruct ss(Name);

  void (SampleStruct::* function)();
  char ch;

  cout << "Please choose action:" << endl << endl;
  cout << "[H] Hello" << endl;
  cout << "[B] Bye" << endl << endl;

  cin >> ch;

  switch (ch)
  {
    case 'H':
    case 'h':
      function = &SampleStruct::printHello;
      break;
    case 'B':
    case 'b':
      function = &SampleStruct::printBye;
      break;
    default:
      function = &SampleStruct::printBadCommand;
      break;
  }

  // Invoke the member function on a particular instance by using .* operator to get the function to bind to the instance of the class.
  (ss.*function)();

  return 0;
}
