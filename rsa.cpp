//C++ program for encryption and decryption
#include<iostream>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<vector>
#include<utility>
#include<random>
#include<tuple>

using namespace std;

int prime(long int); //function to check for prime number
tuple<int, int, int, int> encryption_key();
long int cd(int e, int t);
pair<string,vector<long int>> encrypt(string msg, int e,int n);
string decrypt(string en, int d, int n, vector<long int> temp);
pair<int, int> getXY();

int main()
{
   auto[n,t,e,d] = encryption_key();

   cout << "\nENTER MESSAGE OR STRING TO ENCRYPT\n";
   string msg;
   getline(cin, msg);
   
   auto [en, temp] = encrypt(msg, e, n);
   cout << en << endl;
   cout << decrypt(en, d, n, temp) << endl;
   return 0;
} //end of the main program

pair<int, int> getXY(){
   pair<int, int> xy;
   vector<int> vec;

   std::random_device dev;
   std::mt19937 rng(dev());
   std::uniform_int_distribution<std::mt19937::result_type> dist6(10, 100);
   while (1)
   {
      int num = dist6(rng);
      if(prime(num)){
         if(vec.size() == 1 && num == vec[0]){
            break;
         }
         vec.push_back(num);
         if(vec.size() == 2){
            xy.first = vec[0];
            xy.second = vec[1];
            return xy;
         }
      }
   }
   return xy;
}

int prime(long int pr)
{
   int i;
   int j = sqrt(pr);
   for(i = 2; i <= j; i++)
   {
      if(pr % i == 0)
         return 0;
   }
   return 1;
 }

//function to generate encryption key
tuple<int, int, int, int> encryption_key()
{
   auto [x, y] = getXY();
   int n, t, e, d, flag;
   //checking whether input is prime or not
   flag = prime(x);
   if(flag == 0)
   {
      cout << "\nINVALID INPUT\n";
      exit(0);
   }

   flag = prime(y);
   if(flag == 0 || x == y)
   {
      cout << "\nINVALID INPUT\n";
      exit(0);
   }

   n = x * y;
   t = (x - 1) * (y - 1);

   int k;
   k = 0;
   for(int i = 2; i < t; i++)
   {
      if(t % i == 0)
         continue;
      flag = prime(i);
      if(flag == 1 && i != x && i != y)
      {
         e = i;
         flag = cd(e, t);
         d = flag;
         k++;
         break;
      }
   }

   return make_tuple(n, t, e, d);
}

long int cd(int a, int t)
{
   long int k = 1;
   while(1)
   {
      k = k + t;
      if(k % a == 0)
         return(k/a);
   }
}

//function to encrypt the message
pair<string,vector<long int>> encrypt(string msg, int e,int n)
{
   vector<long int> temp(10000);
   string en = msg;
   int pt, ct, key = e, k, len;
   int i = 0;
   len = msg.length();

   while(i != len)
   {
      pt = msg[i];
      pt = pt - 96;
      k = 1;
      for(int j = 0; j < key; j++)
      {
         k = k * pt;
         k = k % n;
      }
      temp[i] = k;
      ct= k + 96;
      en[i] = ct;
      i++;
   }
   return make_pair(en, temp);
}

//function to decrypt the message
string decrypt(string en, int d, int n, vector<long int> temp)
{
   string msg = en;
   int pt, ct, key = d, k;
   int i = 0;

   while(i != en.length())
   {
      ct = temp[i];
      k = 1;
      for(int j = 0; j < key; j++)
      {
         k = k * ct;
         k = k % n;
      }
      pt = k + 96;
      msg[i] = pt;
      i++;
   }
   return msg;
}