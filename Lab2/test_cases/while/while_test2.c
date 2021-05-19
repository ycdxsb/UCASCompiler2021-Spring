int a = 1;

void f(){
   int b[] = {1,2,3,4,5,6,7,8};
   while(a < 8){
       b[a] = b[a] + 1;
       a = a + 1;
   }
}
