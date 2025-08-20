#include <iostream>
#include <cstring>
// #include "editDistance.h"

#define EDITORDERNONE	0
#define EDITORDERDELETE	1
#define EDITORDERINSERT	2
#define EDITORDEREQUAL	3

using namespace std;

#define DEBUG 0

class EditOrder;

class EditCost {
public:
protected:
	const char* stringSource = nullptr;
	const char* stringTarget = nullptr;
	int _id;
	EditCost* _next = nullptr;
	EditCost* _back = nullptr;
	int cost;
	bool caseInsensitive = false;
	bool glob = false;
public:
	static int counter;
	EditCost(const char*, const char*, int = 0);
        ~EditCost();
#if DEBUG > 0
        int getID();
        const char* getStringSource();
        const char* getStringTarget();
#endif
        void out(); 
	size_t walk();
	void insert(const char*, const char*, int);
	const int getCost();
	const size_t getSourcelen();
	const size_t getTargetlen();
	void setCaseInsensitive(bool);
	bool getCaseInsensitive();
	void setGlob(bool);
	bool getGlob();
	EditCost* getNext();
	void setNext(EditCost* next);
	EditCost* getBack();
	void setBack(EditCost* back);
	EditOrder* createOrder();
};
int EditCost::counter = 0;

/*
 * Hold a list of commands for transforming one string into another
 */
class EditOrder {
public:
	EditOrder* _next = nullptr;
	int order = EDITORDERNONE;
	void out();
	int length();
	char* display(const char*, const char*, int);
};

EditCost::EditCost(const char* source, const char* target, int costPar) {
	stringSource = source;
	stringTarget = target;
	cost = costPar;
	_id = EditCost::counter++;
}

EditCost::~EditCost() {
	if (_next != nullptr) delete _next;
}

#if DEBUG > 0
int EditCost::getID() { return _id; };
const char* EditCost::getStringSource() { return stringSource; }
const char* EditCost::getStringTarget() { return stringTarget; }
#endif
const int EditCost::getCost() { return cost; }
const size_t EditCost::getSourcelen() { return strlen(stringSource); }
const size_t EditCost::getTargetlen() { return strlen(stringTarget); }
void EditCost::setNext(EditCost* next) { _next = next; }
void EditCost::setBack(EditCost* back) { _back = back; }
EditCost* EditCost::getBack() { return _back; }
EditCost* EditCost::getNext() { return _next; }
void EditCost::setCaseInsensitive(bool b) { caseInsensitive = b; }
bool EditCost::getCaseInsensitive() { return caseInsensitive; }
void EditCost::setGlob(bool b) { glob = b; }
bool EditCost::getGlob() { return glob; };

void EditCost::insert(const char* source, const char* target, int costPar) {
	// cout << "EditCost::insert\t" << costPar << "\t" << source << "\t" << target << endl;
    EditCost* walker = this;
	bool b = true;
	while (b) {
		b = false;
		bool addHere = true;
		if (walker->getNext() != nullptr) {
			addHere = false;
			// test if actual costs lower than next element
			if (costPar < walker->getNext()->getCost()) addHere = true;
			else {
				if (costPar == walker->getNext()->getCost()) {
					if (strlen(source) < walker->getNext()->getSourcelen()) addHere = true;
					else {
						if (strlen(source) == walker->getNext()->getSourcelen()) {
							if (strlen(target) < walker->getNext()->getTargetlen()) addHere = true;
							else {
								// if all parameters are equal to the parameter of the walker, do nothing
								if (strlen(target) > walker->getNext()->getTargetlen()) b = true;
							}
						}
						else b = true;
					}
				}
				else b = true;; 
			}
		}
		if (addHere) {
			EditCost* beforeNext = walker->getNext();
			EditCost* n = new EditCost(source, target, costPar);
			n->setCaseInsensitive(this->getCaseInsensitive());
			n->setGlob(this->getGlob());
			n->setBack(this);
			walker->setNext(n);
			n->setNext(beforeNext);
		}
		if (b) {
			walker = walker->getNext();
			if (walker == nullptr) b = false;
		}
	}
}

size_t EditCost::walk() {
	// normally from one Position you can make two steps (insert and delete)
	// if the next char are equal, you can make a step forward with zero costs
	// cout << "EditCost::walk\t" << "\t" << cost << "\t" << stringSource << "\t" << stringTarget << endl;
	size_t result = strlen(stringSource) + strlen(stringTarget) + cost + 1;
	if (strlen(stringSource) > 0) {
		if (strlen(stringTarget) > 0) {
			// equalStep
			if (*stringSource == *stringTarget) {
				insert(stringSource + 1, stringTarget+1, cost);
                        } else if (caseInsensitive && tolower(*stringSource) == tolower(*stringTarget)) {
							insert(stringSource + 1, stringTarget+1, cost);
						}
			// deleteStep
			insert(stringSource + 1, stringTarget, cost + 1);
			// insertStep
			insert(stringSource, stringTarget + 1, cost + 1);
			if (glob) {
				if (*stringSource == '*') {
					insert(stringSource, stringTarget + 1, cost);
					// jump over the wildcard with zero costs
					insert(stringSource + 1, stringTarget, cost);
					// jump over the wildcard and over element
					insert(stringSource + 1, stringTarget+1, cost);
				}
			}
		}
		else {
			if (glob && *stringSource == '*') {
				// trailing wildcard
				insert(stringSource + 1, stringTarget, cost);
			}
			else {
				// deleteStep
				insert(stringSource + 1, stringTarget, cost + 1);
			}
		}
	}
	else {
		if (strlen(stringTarget) > 0) {
			// insertStep
			insert(stringSource, stringTarget + 1, cost + 1);
		}
	}
	if (strlen(stringSource) == 0 && strlen(stringTarget) == 0) result = cost;
	// cout << "EditCost::walk\t"  << result << "\t" << cost << "\t" << stringSource << "\t" << stringTarget << endl;
	return result;
}

void EditCost::out() {
	cout << "EditCost:\t" << cost << "\t" << stringSource << " <> " << stringTarget << endl;
}

EditOrder* EditCost::createOrder() {
	cout << "EditCost::createOrder:\t" << endl;
	EditOrder* result = nullptr;
	EditOrder* resultNext = nullptr;
	EditCost* walker = this;
	EditCost* walkerNext;
	while (walker != nullptr) {
		cout << "EditCost::createOrder:\t";
		walkerNext = walker->getBack();
		if (walkerNext != nullptr) {
			resultNext = result;
			if (walkerNext->getSourcelen() > walker->getSourcelen()) {
				if (walkerNext->getTargetlen() > walker->getTargetlen()) {
					result = new EditOrder();
					result->_next = resultNext;
					result->order = EDITORDEREQUAL;
					resultNext = nullptr;
					cout << "=";
				}
				else {
					result = new EditOrder();
					result->_next = resultNext;
					result->order = EDITORDERDELETE;
					resultNext = nullptr;
					cout << "d";
				}
			}
			else {
				if (walkerNext->getTargetlen() > walker->getTargetlen()) {
					result = new EditOrder();
					result->_next = resultNext;
					result->order = EDITORDERINSERT;
					resultNext = nullptr;
					cout << "i";
				}
				else {
					cout << "?";
				}
			}
		}
		cout << "\t" << walker->getSourcelen() << " " << walker->getTargetlen() << "\t" << walker->_id << endl;
		walker = walkerNext;
	}
	return result;
}

void EditOrder::out() {
    cout << "EditOrder:\t";
    EditOrder* walker = this;
    while (walker != nullptr) {
        if (walker->order == EDITORDERNONE) {
            cout << "-";
        } else if (walker->order == EDITORDERDELETE) {
            cout << "d";
        } else if (walker->order == EDITORDERINSERT) {
            cout << "i";
        } else if (walker->order == EDITORDEREQUAL) {
            cout << "=";
        } else {
            cout << "?";
        }
        walker = walker->_next;
    }
    cout << endl;
}

int EditOrder::length() {
	int result = 0;
    EditOrder* walker = this;
    while (walker != nullptr) {
		result++;
		walker = walker->_next;
	}
	return result;
}

char* EditOrder::display(const char* source, const char* target, int index) {
	cout << "EditOrder:\t" << index << "\t";
	size_t indexSource = 0;
	size_t indexTarget = 0;
    EditOrder* walker = this;
	char* result = new char[255];
	for (int i = 0; i <= index; i++) {
		if (walker->order == EDITORDERINSERT) {
			indexTarget++;
        } else if (walker->order == EDITORDERDELETE) {
            indexSource++;
        } else if (walker->order == EDITORDEREQUAL) {
            indexSource++;
			indexTarget++;
        } else {
			cout << " wtf ";
		}
		walker = walker->_next;
	}
	
	delete[] result;
    cout << "\t" << indexSource << " - " << indexTarget << "\t";
	for (size_t k1 = 0; k1 < indexTarget; k1++) {
		cout << target[k1];
	}
	cout << "#";
	for (size_t k2 = indexSource; k2 < strlen(source); k2++) {
		cout << source[k2];
	}
	cout << endl;
	return nullptr;
}

/**
* global function
*/
int editDistance(const char* sSource, const char* sTarget, bool caseInsensitive=false, bool glob=false) {
#if DEBUG > 0
    cout << "=editDistance\tid\trest\tcost\tpattern\ttarget\tprevious" << endl;
#endif
	int best = strlen(sSource) + strlen(sTarget) + 999;
	// EditCost is a Queue sorted on the editdistance (=costs) for a step
	EditCost* e = new EditCost(sSource,sTarget);
	e->setCaseInsensitive(caseInsensitive);
	e->setGlob(glob);
	EditCost* walker = e;
	while (walker != nullptr) {
		int value = walker->walk();
#if DEBUG > 0
		cout << "editDistance:\t" << walker->getID() << "\t" << value << "\t" << walker->getCost() << "\t" << walker->getStringSource() << "\t" << walker->getStringTarget();
		if (walker->getBack() != nullptr) {
			cout << "\t" << walker->getBack()->getID();
		}
		cout << endl;
#endif
		if (value < best) {
			best = value;
		}
#if DEBUG > 0
                if (value == best && walker->getSourcelen() == 0 && walker->getTargetlen() == 0) {
                    walker->out();
                    EditOrder* m = walker->createOrder();
                    m->out();
                    for (int i = 0; i < m->length(); i++) {
                        m->display(sSource, sTarget, i);
                    }
                }
#endif
                walker = walker->getNext();
		if (best < walker->getCost()) {
			walker = nullptr;
		}
	}
        
#if 0
	cout << "EditCostStack:" << endl;
	// EditCost* walker = e;
	EditCost* w = e;
	while (w != nullptr) {
		w->out();
		w = w->getNext();
	}
#endif
	delete e;
	return best;
}

int editDistanceI(const char* sSource, const char* sTarget) {
	return editDistance(sSource, sTarget, true);
}

int editDistanceGlob(const char* sSource, const char* sTarget) {
	return editDistance(sSource, sTarget, false, true);
}

