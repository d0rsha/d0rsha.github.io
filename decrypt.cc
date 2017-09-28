#include <iostream>
#include <chrono>
#include "Key.h"
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdint.h> 
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

using namespace std;
namespace std{
   template<> struct
   hash<Key> {
      std::size_t operator()(const Key & k) const
         {
            string s_data;
            for ( auto tecken : k.digit )
            {  
               s_data += tecken;
            }
            int length{C};
            int rem{C & 3};
            size_t hash = length, tmp;
            const char * data = s_data.c_str();
             
            length >>= 2;
             
            for (;length > 0; length--) {
               hash += get16bits(data);
               tmp = (get16bits (data+2) << 11) ^hash;
               hash = (hash << 16) ^tmp;
               data += 2*sizeof(uint16_t);
               hash += hash >> 11;
            }
            
// Handle end cases 
            switch (rem) {
            case 3: hash += get16bits (data);
               hash ^= hash << 16;
               hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
               hash += hash >> 11;
               break;
            case 2: hash += get16bits (data);
               hash ^= hash << 11;
               hash += hash >> 17;
               break;
            case 1: hash += (signed char)*data;
               hash ^= hash << 10;
               hash += hash >> 1;
            }
 // Force "avalanching" of final 127 bits 
            hash ^= hash << 3;
            hash += hash >> 5;
            hash ^= hash << 4;
            hash += hash >> 17;
            hash ^= hash << 25;
            hash += hash >> 6;

            return hash;
         }
/* Alternativ funktion 
auto tecken = k.digit[i];
               sum += (tecken % 1024);
               tecken >> 2;
               sum *= (tecken % 1024) +1;
               tecken >> 2;
               sum *= tecken +1;
                //shifta tecken till vänster
                // så att det blir en jämn fördelning av höga och låga tal
                //Dvs fråga Pontus
            }
            return sum ;//% (long int)pow(2,(int)(N/2)+2);
*/
   };
}
 
int main(int argc, char* argv[])
{
  unsigned char buffer[C+1];     // temporary string buffer
  Key encrypted;                 // the encrypted password
  Key zero = {{0}};              // the all zero key
  Key T[N];                      // the table T
 
  unordered_map<Key,vector<Key>> ht;
  
  if (argc != 2)
  {
     cout << "Usage:" << endl << argv[0] << " password < rand" << C << ".txt" << endl;
 
     return 1;
  }
  // read in table T
  for (int i{0}; i < N; ++i)
  {
     scanf("%s", buffer);
     T[i] = KEYinit(buffer);
  }

  encrypted = KEYinit((unsigned char *) argv[1]);
  auto begin = chrono::high_resolution_clock::now();

  // Create a key that is equal to 2^(N/2)
  Key middle{{0}};
  middle++;
  for (int i{}; i < N/2; i++)
      middle = middle + middle;
 
  // Let counter go from 0 -> middle
  // Save the counter value in the searchkey E - subsetsum
  Key counter{{0}};
  while ( counter != middle )
  {
      ht[encrypted - KEYsubsetsum(counter, T)].push_back(counter); // kollisionshantering
      counter++;
  }
 
  Key past_middle_sum{{0}};
 
  //Iterate thru the bits of the higher half of the table
  do //Därför blir antalet fall nu 2^(N/2) * 2
  {
      Key candidate = KEYsubsetsum(past_middle_sum, T);
      unordered_map<Key,vector<Key>>::iterator it = ht.find(candidate);
      if ( it != ht.end() )
      {
          for ( Key match : ht[candidate] )
              cout << past_middle_sum + match << endl;
      }
      past_middle_sum = past_middle_sum + middle;
  } while ( past_middle_sum != zero );
 
  auto end = chrono::high_resolution_clock::now();
  cout << "Decryption took "
       << std::chrono::duration_cast<chrono::seconds>(end - begin).count()
       << " seconds." << endl;
 
  return 0;
}
