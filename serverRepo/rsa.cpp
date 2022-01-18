//C++ program for encryption and decryption
#include<iostream>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<vector>
#include<utility>
#include<random>
#include<tuple>

int prime(long int); //function to check for prime number
std::tuple<int, int, int, int> encryption_key();
long int cd(int e, int t);
std::string encrypt(std::string msg, int e,int n);
std::string decrypt(std::string en, int d, int n);
std::pair<int, int> getXY();
std::vector<std::string> Split(std::string str, std::string delimiter);

// int main()
// {
//    auto[n,t,e,d] = encryption_key();

//    std::cout << "\nENTER MESSAGE OR STRING TO ENCRYPT\n";
//    std::string msg;
//    getline(std::cin, msg);
   
//    std::string en = encrypt(msg, e, n);
//    std::cout << en << std::endl;
//    std::cout << decrypt(en, d, n) << std::endl;
//    return 0;
// } //end of the main program

std::pair<int, int> getXY(){
   std::pair<int, int> xy;
   std::vector<int> vec;

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
            xy.first = 11;
            xy.second = 13;
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
std::tuple<int, int, int, int> encryption_key()
{
   auto [x, y] = getXY();
   int n, t, e, d, flag;
   //checking whether input is prime or not
   flag = prime(x);
   if(flag == 0)
   {
      std::cout << "\nINVALID INPUT\n";
      exit(0);
   }

   flag = prime(y);
   if(flag == 0 || x == y)
   {
      std::cout << "\nINVALID INPUT\n";
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

   return std::make_tuple(n, t, e, d);
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
std::string encrypt(std::string msg, int e,int n)
{
   std::vector<long int> temp(msg.length());
   std::string en = msg;
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

   en += "@";
   for(int i = 0; i < temp.size(); i++){
      en += (std::to_string(temp[i]));
      if(i != temp.size() - 1){
         en += ",";
      }
   }

   return en;
}

//function to decrypt the message
std::string decrypt(std::string en, int d, int n)
{
   std::string tempstr = en.substr(en.find('@') + 1);
   en = en.substr(0, en.find('@'));
   std::vector<long int> temp;
   std::string msg = en;
   int pt, ct, key = d, k;

   std::vector<std::string> tmpList = Split(tempstr, ",");
   for(std::string s : tmpList){
      temp.push_back(stoi(s));
   }

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

// std::vector<std::string> Split(std::string str, std::string delimiter){
//     std::vector<std::string> out;

//     size_t pos = 0;
//     std::string token;
//     while ((pos = str.find(delimiter)) != std::string::npos) {
//         token = str.substr(0, pos);
//         out.push_back(token);
//         str.erase(0, pos + delimiter.length());
//     }
//     out.push_back(str);
//     return out;
// }