#include <stdio.h>
#include <math.h>
#include <sndfile.h>
#include <string.h>
#include <stdlib.h>


int bucket_size = 1000;
int samplerate = 44100;//sample rate in hz



double bucket_to_freq(double * data, int len){
//len may not be BUCKET_SIZE

double* datatwo = (double*)malloc(len* sizeof(double)); 
int* datapeaks = (int*)malloc(len* sizeof(int)); 

//auto correllation, kind of
int i;
int t;
double meandat = 0;///mean of actual data
double mean = 0; ///mean of autocoreelation data

for( i = 0; i < len; i++){
meandat += data[i];
}meandat = meandat/len;

for( i = 0; i < len; i++){

double sum = 0;
double sumb = 0;
for( t = 0; t< len; t++){
if(data[t]-meandat == 0){//avoid divide by zero 	
sum+=((data[(i+t)%(len)]-meandat)*(data[t]-meandat));
sumb+=((data[t]-meandat)*(data[t]+0.0001-meandat));
}else{
sum+=((data[(i+t)%(len)]-meandat)*(data[t]-meandat));
sumb+=((data[t]-meandat)*(data[t]-meandat));
}


}
datatwo[i] = sum/sumb;

mean += datatwo[i];

}
mean = mean/(double)len;
////peak detection
int peakindex = 0;
int dataindex = 0;
//calculate standard deviation of datatwo
double standev =0;
for( i = 0; i < len; i++){
standev += (datatwo[i]-mean)*(datatwo[i]-mean);
}
standev = standev/len;
standev = sqrt(standev);


for(i = 1; i < len-1; i++){
if(datatwo[i] > datatwo[i-1] && datatwo[i] > datatwo[i+1] && datatwo[i] > mean + standev*0.5)
{
datapeaks[dataindex] = i;
dataindex++;
}
}


///return freq
if(dataindex == 0){
///no peaks
return 1;
}

printf("%d \n",datapeaks[0]);

int d = datapeaks[0];

free(datatwo);
free(datapeaks);

return 1.0/(d/(double)samplerate);
}


int freq_to_notenumber(double freq){
///// based on A = 440hz
////reverse intreval to freq based on -1 octave and then intelligently round to get number of steps up from A-1
if(freq == 1){
return -1;
}
return round(12* log2( freq / 13.75));
}


int notenumber_to_noteoctave(int num){
/////divide by 12 and floor to get rid of remainder, remember note o is A-1 so subtract 1
if(num == -1){
return 0;
}
return floor(num/12)-1;
}

///lowercase for the flats so no dynamic string arrays later on
char* notestring[13] = {"a","bes","b","c","des","d","ees","e","f","ges","g","aes","r"};

int notenumber_to_noteletter(int num){
///mod 12 and then use conversion array
if(num == -1){
return 12;
}
return num%12;
}



int main(int argc, char* argv[]){

int noteletter;//index of notestring
int noteoctave;
int notenum;

SNDFILE *wavtoread;

SF_INFO sfinfo ;
int readcount ;
float buckfreq;
int tempo = 100; //tempo in bpm
memset(&sfinfo, 0, sizeof (sfinfo)) ;



if(argc <= 1){
printf("No file given.");
return 0;
}

if(argc == 3){
////convert string to int for projected tempo
tempo = atoi(argv[2]);
}
if (! (wavtoread = sf_open (argv[1], SFM_READ, &sfinfo))){      
                puts (sf_strerror (NULL));
                return 0;
}

samplerate = sfinfo.samplerate;
bucket_size = (int)samplerate/30; /// 30 notes per second should be more than sufficient for all but the fastest of players



double* data = (double*)malloc(bucket_size*sizeof(double));

int* octaves = (int*)malloc((int)(1+sfinfo.frames/bucket_size)*sizeof(int));
int ind = 0; //len of completed notes arrays
int* notes = (int*)malloc((int)(1+sfinfo.frames/bucket_size)*sizeof(int));

printf("\n  FRAMES( number of samples ) %ld at sample rate %d \n",sfinfo.frames,samplerate);


 while ((readcount = sf_read_double (wavtoread, data, bucket_size))){

buckfreq =  bucket_to_freq(data, readcount);
notenum = freq_to_notenumber(buckfreq);
noteletter = notenumber_to_noteletter(notenum); 
noteoctave = notenumber_to_noteoctave(notenum);

printf("%f   ",buckfreq);
printf("%s%d \n",notestring[noteletter], noteoctave);
octaves[ind] = noteoctave;
notes[ind] = noteletter;
ind++;
////get freq convert to note and store in notes array
//but for now just print it out


}

/////now connect the dots



///open a file to output for lilypond to engrave
///argv[1]
char outname[strlen(argv[1]+1+4)];
strcat(outname,argv[1]);
strcat(outname,".ly");

FILE* outwrite = fopen(outname,"w");
if(outwrite == NULL){
printf("Failed to Write File");
return 0;
}

fprintf(outwrite, "{  \n \\clef treble \n");


int slots = 0;
double duration = 0.0;
int index = 0;
while(index<ind){
while(index < ind-1 && notes[index] == notes[index+1] && octaves[index] == octaves[index+1]){
slots++;
index++;
}
if( octaves[index] > 8){
index++;
continue;
}


duration = (double)(slots*bucket_size)/(double)(samplerate);//in seconds per note 
double beatspersecond = (double)(tempo)/60.0;


duration = duration*beatspersecond;////in beats/note
//printf("beats at tempo (default tempo is 100bpm)  %f \n",duration);
slots = 0;


int value = 0;
int time=0;
while(duration > 0){

if(duration >=4 ){
value = 1;duration-=4;
}else if(duration >=2){
value = 2; duration-=2;
}else if(duration >= 1){
value = 4; duration-=1;
}else if(duration >= 0.5){
value = 8; duration-=0.5;
}else if(duration >=0.25){
value = 16; duration -=0.25;
}else if(duration >=0.125){
value = 32; duration -=0.125;
}else{
value = 64; duration = 0;
}

//arbitrary cutofff tempo for very very fast notes unlikley to be played at that tempo
if(tempo > 85 && value < 64){
 
fprintf(outwrite, "%s",notestring[notes[index]]);

if(notes[index] != 12){
int oct = octaves[index] - 2;
if(notes[index] <= 2){
oct--;
}
while(oct > 0){
fprintf(outwrite,"'");

oct--;
}
while(oct < 0){
fprintf(outwrite,",");

oct++;
}


}


fprintf(outwrite,"%d",value);
}


if(duration > 0 && time == 0){
///handle beaming ties
fprintf(outwrite, "(");
}
if(duration == 0 && time > 0){
fprintf(outwrite, ")");
}
time++;
}
fprintf(outwrite, "\n");

index++;
///output to lilypond here
}




fprintf(outwrite, "}");

sf_close (wavtoread);
fclose(outwrite);

free(notes);
free(octaves);

return 0;
}



