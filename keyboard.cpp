#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <iterator>
using namespace std;

//class for each learned word
class Candidate {
public:
  Candidate(); //constructor
  void setWord(string word); //setter for word
  void incrementConfidence(); //increment confidence level
  string getWord(); //getter for word
  int getConfidence(); //getter for confidence level
private:
  string m_word; //stores the word
  int m_confidence; //stores the confidence level
};

//class for list in keyboard
class List {
  struct Node {
    map<string, Node*> m_data; //stores the letter and Node for the next letter
    Candidate m_word; //stores the word
  };
public:
  List(); //constructor
  ~List(); //destructor
  void recurseDelete(Node* curr); //recursive destructor
  void add(string word); //add word to list
  list<Candidate> find(string fragment); //returns a list of words containing the beginning fragment
  Candidate recurse(Node* curr, list<Candidate> &found); //add each word after curr to found
  int getMax(); //getter for max
  void setMax(int max); //setter for max
  list<Candidate> dump(); //debug
private:
  Node* m_head; //head of linked list
  int m_max; //stores max confidence level
};

//class for keyboard
class AutocompleteProvider {
public:
  AutocompleteProvider(); //constructor
  ~AutocompleteProvider(); //destructor
  list<Candidate> findWords(string fragment); //returns list of candidates ordered by confidence
  void train(string passage); //trains the algorithm with the provided passage
  void print(list<Candidate> provider); //print the words in the provider for autocomplete suggestions
  void dump(); //debug
private:
  List* m_keyboard; //linked list keyboard
};

//constructor
Candidate::Candidate() {
  m_word = "";
  m_confidence = 1;
}

//setter for word
void Candidate::setWord(string word) {
  m_word = word;
}

//increment confidence level
void Candidate::incrementConfidence() {
  m_confidence += 1;
}

//getter for word
string Candidate::getWord() {
  return m_word;
}

//getter for confidence level
int Candidate::getConfidence() {
  return m_confidence;
}

//constructor
AutocompleteProvider::AutocompleteProvider() {
  m_keyboard = new List();
}

//destructor
AutocompleteProvider::~AutocompleteProvider() {
  delete m_keyboard;
  m_keyboard = NULL;
}

//find words beginning with fragment in keyboard
list<Candidate> AutocompleteProvider::findWords(string fragment) {
  list<Candidate> found;
  found = m_keyboard->find(fragment);
  return found;
}

//train the keyboard to learn each word in the passage
void AutocompleteProvider::train(string passage) {
  string word = "";
  for (int i = 0; i < passage.length(); i++) {
    //iterate to a word
    if (passage.substr(i, 1) != " " and passage.substr(i, 1) != ".") {
      word += passage.substr(i, 1);
    }
    //add word to keybaord
    else {
      boost::algorithm::to_lower(word);
      m_keyboard->add(word);
      word = "";
    }
  }
  m_keyboard->add(word);
}

//print the words in the provider for autocomplete suggestions
void AutocompleteProvider::print(list<Candidate> provider) {
  int count = m_keyboard->getMax();
  int keep = 0;
  list<Candidate>::iterator it;
  it = provider.begin();
  //iterate through provider and print from highest to lowest confidence
  while (it != provider.end() and count > 0) {
    if (it->getConfidence() == count) {
      cout << "\"" << it->getWord() << "\" (" << it->getConfidence() << ")";
      if (provider.size() != 1) {
	cout << ", ";
      }
      provider.erase(it);
      it = provider.begin();
      advance(it, keep);
    }
    else {
      keep += 1;
      it++;
    }
    if (provider.size() != 0 and it == provider.end()) {
      it = provider.begin();
      count -= 1;
      keep = 0;
    }
  }
  cout << endl;
}

//debug
void AutocompleteProvider::dump() {
  list<Candidate> found;
  found = m_keyboard->dump();
  print(found);
}

//constructor
List::List() {
  m_head = NULL;
  m_max = 1;
}

//destructor
List::~List() {
  if (m_head != NULL) {
    recurseDelete(m_head);
  }
}

//recursive destructor
void List::recurseDelete(Node* curr) {
  map<string, Node*>::iterator it;
  it = (curr->m_data).begin();
  //base case
  if (it->second == NULL) {
    delete curr;
    return;
  }
  //recursive case
  while (it != (curr->m_data).end()) {
    recurseDelete(it->second);
    it++;
  }
  delete curr;
}

//add word to list
void List::add(string word) {
  //if list is empty
  if (m_head == NULL) {
    Node* curr = new Node();
    m_head = curr;
  }
  Node* curr = m_head;
  Node* prev;
  bool newWord = false;
  //add each letter to list
  for (int i = 0; i < word.length(); i++) {
    prev = curr;
    string letter = word.substr(i, 1);
    curr = curr->m_data.find(letter)->second;
    //if letter was not added to path yet
    if (curr == NULL) {
      newWord = true;
      curr = new Node();
      prev->m_data.insert(std::pair<string, Node*>(letter, &*curr));
    }
  }
  //if not a repeated word
  if (newWord) {
    curr->m_word.setWord(word);
  }
  //if a repeated word
  else {
    curr->m_word.incrementConfidence();
    if (curr->m_word.getConfidence() > m_max) {
      setMax(curr->m_word.getConfidence());
    }
  }
}

//returns a list of words containing the beginning fragment
list<Candidate> List::find(string fragment) {
  int i = 0;
  Node* curr = m_head;
  Node* prev;
  //iterate to position of fragment in list
  while (i < fragment.length() and curr != NULL) {
    prev = curr;
    string letter = fragment.substr(i, 1);
    curr = (curr->m_data).find(letter)->second;
    i += 1;
  }
  list<Candidate> found;
  if (curr != NULL) {
    if (curr->m_word.getWord() != "") {
      found.push_back(curr->m_word);
    }
    recurse(curr, found);
  }
  return found;
}

//add each word after curr to found
Candidate List::recurse(Node* curr, list<Candidate> &found) {
  map<string, Node*>::iterator it;
  it = (curr->m_data).begin();
  //base case
  if (it->second == NULL) {
    return curr->m_word;
  }
  //recursive case
  while (it != (curr->m_data).end()) {
    Candidate word = recurse(it->second, found);
    if (word.getWord() != "") {
      found.push_back(word);
    }
    it++;
  }
  return curr->m_word;
}

//getter for max
int List::getMax() {
  return m_max;
}

//setter for max
void List::setMax(int max) {
  m_max = max;
}

//debug
list<Candidate> List::dump() {
  list<Candidate> found;
  recurse(m_head, found);
  return found;
}

int main() {
  string input = "";
  cout << "Enter 1 to train, 2 to input, 0 to exit: ";
  cin >> input;
  AutocompleteProvider* keyboard = new AutocompleteProvider();
  while (input != "0") {
    //if user wants to train the keyboard
    if (input == "1") {
      string passage = "";
      cout << "Train: ";
      cin.ignore();
      getline(cin, passage, '\n');
      //train input
      keyboard->train(passage);
    }
    //if user wants to get autocomplete suggestions
    else if (input == "2") {
      list<Candidate> provider;
      string fragment = "";
      cout << "Input: ";
      cin >> fragment;
      boost::algorithm::to_lower(fragment);
      //find words based on input
      provider = keyboard->findWords(fragment);
      if (provider.empty()) {
	cout << "No words trained to match the fragment." << endl;
      }
      else {
	cout << "\"" << fragment << "\" --> ";
	keyboard->print(provider);
      }
    }
    cout << "Enter 1 to train, 2 to input, 0 to exit: ";
    cin >> input;
  }
  delete keyboard;
}
