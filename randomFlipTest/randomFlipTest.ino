//a test of the emperical frequency of random coin flips based on an analog pin

//the number of flips each round (batchSize) for a set number of rounds (numberOfSims)
const long int batchSize = 200;
const long int numberOfSims = 1000;

//initialize arrays
int flips[batchSize]={};                //array for a single rounds flips (continually overwritten)
double aggregateAvgs[numberOfSims]={};  //array to store the averages from every round

long loopCount = 0;

//a function to calculate the average value of a given round
double average(){
  double sum = 0;
  for(int i = 0; i < (batchSize+1); i++){
    sum = sum + flips[i];
  }
  return sum/batchSize;
}

/*
 * print values to see them, but seems if every flip is printed the heavy serial monitor prints affect outcome slightly...
 * E.x. using randomSeed(analogRead(A0));
 * printing every flip = ~.63 aggregate average or more
 * printing only every round average = ~.53 aggregate average
 *  
 * Using the pin in the farthest corner of chip, A15. The same difference is observed. 
 * Very curious about the exact source of this.
 * 
 * At the very least, and if storage allows, recording the random numbers generated every loop while a full project is running would
 * give a more accurate/specific expected frequency of 'random' values.
 * 
 * A slight positive bias toward random() returning '1' could perhaps be a good thing. This could correspond to the correct symbol 
 * being shown slightly more often, which helps set the cat up for wins more often. I've noticed that in training, if a cat is having
 * trouble understanding a new level of a target behavior progression, it helps to go back, simplify what is being asked of them, 
 * and set them up for success. By slowly shaping the behavior with smaller measures of progress (and often more precise timing of clicks/cues)
 * you can move forward with them incrementally and eventually try only rewarding the more complex behavior once again. 
 * 
 * I guess the risk is that the higher this success rate becomes, the more the chance of 'false rewards' (or rewards got by 
 * randomly smashing the button regardless of symbol shown) also increases. This might impede the learning of actual symbol 
 * discrimination but perhaps at some level it is worth the sacrifice (esp. if the sacrifice is only one of 'learning rate')
 * for increased engagement/success when it comes to cat training in the long run. 
 */
 
void setup() {
  Serial.begin(9600);
}

void loop() {
  //get a new seed every loop based on random analog pin noise
  randomSeed(analogRead(A15));

  //perform a single rounds worth of flips 
  for(int i = 0; i< (batchSize + 1);i++){
    flips[i] = random(0,2);
    //Serial.println(flips[i]); //don't do this unless you want to spike the frequency of getting a '1' 10% apparently
  }

//  Serial.println("");
//  Serial.print("Average: ");
//  Serial.print(average());
//  Serial.println("");

  //store each round average
  if(loopCount<= numberOfSims){
    aggregateAvgs[loopCount] = average();
  }else{
    Serial.println("-----------------------------------------");
    double sum = 0;
    for(int i = 0; i < numberOfSims + 1; i++){
      sum = sum + aggregateAvgs[i];
    }
    double average2 = sum/numberOfSims;
    Serial.print("Total sum of averages: ");
    Serial.print(sum);
    Serial.print('\n');
    Serial.print("Number of sims: ");
    Serial.print(numberOfSims);
    Serial.print(", at batch size: ");
    Serial.print(batchSize);;
    Serial.print('\n');
    Serial.print("Average across sims: ");
    Serial.print(average2);
    Serial.print('\n');
    Serial.println("-----------------------------------------");
    Serial.end();
  }
  loopCount++;
}
