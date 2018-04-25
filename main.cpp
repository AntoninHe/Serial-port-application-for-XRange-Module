#include <fstream>      // std::fstream
#include <iostream>     // std::cout
#include <unistd.h>
#define LENGTH 500
 
#define MSG_YES 'Y'
#define MSG_NO 'N'
#define MSG_AVAILABLE 'D'
#define MSG_ACKNOWLEGDE 'A'
#define MSG_PC_NEW_DATA 'P'

using namespace std;

int main(){
    fstream fs;
    fs.open ("/dev/ttyACM1");
    //fs.open ("test.txt");
    bool newMsg = true; 
    if (fs.is_open())
    {
        cout << "Serial port successfully opened" << endl;
        //usleep(10*1000);
        char c;
        string myString;
        //fs.ignore(10000);
        /*while(fs){
            fs.get(c);
            cout << fs.tellg() << endl;
            cout << "e" << endl;
        }
        usleep(10*1000*1000);
        */
        cout << fs.tellg() << endl;
        fs >> myString;
        c = myString[0];
        myString.clear();
        cout << c << endl;


        if(c == MSG_AVAILABLE)
        {
        //usleep(10*1000);
        
            cout << fs.good() << " good numéro 0" << endl;
            if(newMsg == true){
                //fs.ignore(500);
                fs << MSG_PC_NEW_DATA << flush;
                fs.sync();
            }
            else{
                //fs.ignore(500);
                fs << 'r';
            } 

                //fs.ignore(500);
            //:fs.clear();
            cout << fs.bad() << " bad numéro 1" << endl;
            cout << fs.good() << " good numéro 1" << endl;
            cout << fs.eof() << " eof numéro 1" << endl;

            if(c == MSG_YES){
                cout << "Yes" << endl;
                fs >> myString;
                cout << myString << endl;
            }
            else if(c == MSG_NO){
                cout << "NO";
            }
            else{
                fs.ignore(500);
                cout << c  << endl;
                cout << "error" << endl;

                fs.close();
                return -1;
            }
            if(newMsg == true){
                cout << "msg to send" << endl;
                fs << "ADD5512S";
            }
            else{
                cout << "no msg to send" << endl;
            }
        }


        //cin >> myInput;
        /*while(fs){

            cout << endl << myInput << endl;

            cout << myInput;

            if(fs.fail()){
                cout << "fail" << endl;
                if(fs.bad()){
                    cout << "bad" << endl;
                }
                fs.clear();
            }
        }*/
        fs.close();
    }
    else
    {
            cout << "Error opening file" << endl;
    }
    return 0;
}
