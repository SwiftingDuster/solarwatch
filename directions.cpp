#include <iostream>

using namespace std;

int main()
{
    float azimuth;

    cout << "enter value of azimuth: ";
    cin >> azimuth;


    //in arduino:
    //0 to 22.4, 360 is north
    if ((azimuth >= 0 && azimuth < 22.5) || azimuth == 360)
    {
        cout<<"North";
    }
    //22.5 to 44.9 is north-northeast
    else if (azimuth < 45)
    {
        cout<<"North-Northeast";
    }
    //45 to 67.4 is northeast
    else if (azimuth < 67.5)
    {
        cout<<"Northeast";
    }
    //67.5 to 89.9 is east-northeast
    else if (azimuth < 90)
    {
        cout<<"East-Northeast";
    }


    //90 to 112.4 is east
    else if (azimuth < 112.5)
    {
        cout<<"East";
    }
    //112.5 to 134 is east-southeast
    else if (azimuth < 135)
    {
        cout<<"East-Southeast";
    }
    //135 to 157.4 is northeast
    else if (azimuth < 157.5)
    {
        cout<<"Southeast";
    }
    //157.5 to 179.9 is east-northeast
    else if (azimuth < 180)
    {
        cout<<"South-Southeast";
    }


    //180 to 202.4 is south
    else if (azimuth < 202.5)
    {
        cout<<"South";
    }
    //202.5 to 224.9 is south-southwest
    else if (azimuth < 225)
    {
        cout<<"South-Southwest";
    }
    //225 to 247.4 is southwest
    else if (azimuth < 247.5)
    {
        cout<<"Southwest";
    }
    //247.5 to 269.9 is west-southwest
    else if (azimuth < 270)
    {
        cout<<"South-Southeast";
    }


    //270 to 292.4 is west
    else if (azimuth < 292.5)
    {
        cout<<"West";
    }
    //292.5 to 314.9 is west-northwest
    else if (azimuth < 315)
    {
        cout<<"West-northwest";
    }
    //315 to 337.4 is northwest
    else if (azimuth < 337.5)
    {
        cout<<"Northwest";
    }
    //337.5 to 359.9 is north-northwest
    else if (azimuth < 360)
    {
        cout<<"North-Northwest";
    }

    return 0;
}
