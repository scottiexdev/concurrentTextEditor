#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <QUuid>
#include <QObject>

class Identifier
{
public:

    //Costruttore
    Identifier(int digit, QUuid siteID): _digit(digit), _siteID(siteID){ }

    Identifier(){}

    int _digit;
    QUuid _siteID;

    int compareTo(Identifier otherId) {

      if (this->_digit < otherId._digit) {
        return -1;
      }
      else if (this->_digit > otherId._digit) {
        return 1;
      }
      else {
        if (this->_siteID < otherId._siteID) {
          return -1;
        }
        else if (this->_siteID> otherId._siteID) {
          return 1;
        }
        else {
          return 0;
        }
      }
    }
};


#endif // IDENTIFIER_H
