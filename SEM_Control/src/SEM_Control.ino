/* SEM_Control 
* Arduino code to handle:
*	- Opening and closing the purge valve periodically
*	- Reading of mean temperature data from 4 thermistors
*		- Storing these values to SD card
*		- Sending these values over XBee to another Arduino
*	- Using temperature data to linearly control the fan system (via a PWM fan controller)
*	- Reading battery voltages from I2C (another system does the measurement)
*	- Updating the HUD.
*	
* Dylan Auty, 23 June 2016
*/

// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#include <I2C.h>	// Alternative library for I2C communication
					// dsscircuits.com/articles/arduino-i2c-master-library
#include <SPI.h>
#include <SD.h>		// SD card library includes
#include "Comms.h"	// HUD driver includes


// DEFINE PINS
#define PURGE_PIN 13		// Any digital IO pin.
#define FAN_CONTROL_PIN 45	// Connect Tx pin of a serial port to S1 on Syren 10a. Choices are 1, 18, 16 and 14 on mega2560.
#define THERMISTOR_IN_1 A0	// These 5 for analog input pins from the thermistors.
#define THERMISTOR_IN_2 A1
#define THERMISTOR_IN_3 A2
#define THERMISTOR_IN_4 A3
#define THERMISTOR_IN_5 A4
#define I2C_SDA 20			// Pins for I2C communication
#define I2C_SCL 21
#define THROTTLE_SENSE_PIN 7

// DEFINE RUNNING PARAMETERS/SETTINGS
// Fuel cell purge valve timing
#define PURGE_CLOSED_SECONDS 5 					// Purge valve times only accurate to increments of 0.5 seconds.
#define PURGE_OPEN_SECONDS 3

// Fuel cell temp measurement and control
#define FUEL_CELL_IDEAL_TEMP 40					// In degrees C
#define FUEL_CELL_INDIVIDUAL_MAX_TEMP 60		// If any one temp sensor reads above this, fans will output full..
#define FUEL_CELL_STANDARD_FANSPEED 170 		// This on a scale of 0-255 for full stop to full go.
#define FUEL_CELL_MIN_FANSPEED 50				// Fan speed will not drop below this.
#define FUEL_CELL_FAN_CONTROL_SENSITIVITY 8.5	// In speed units (0-255) per degree C.

// Setup I2C addresses
// Forbidden addresses: 0 to 7, 120 to 127.
#define BMS_VOLTAGE_1_ADDR 85 	// Test runs with address 85
#define BMS_VOLTAGE_2_ADDR 86
#define BMS_VOLTAGE_3_ADDR 87
#define BMS_VOLTAGE_4_ADDR 88
#define BMS_VOLTAGE_5_ADDR 89
#define BMS_VOLTAGE_6_ADDR 90
#define BMS_VOLTAGE_7_ADDR 91
#define BMS_VOLTAGE_8_ADDR 92
#define BMS_VOLTAGE_9_ADDR 93
#define GPS_SHIELD_ADDR 104		// With both jumpers on GPS shield in, address is 104
#define HUD_ADDR 8				

#define BMS_REQUEST_TIMEOUT 40			// Timeout for BMS voltage sensor poll in ms.

// GPS coordinates for the corners of the map
////

// Global variables for valve timing, accessible from ISR.
volatile int time1Counter = 0;		// Timer 1, to check time between purge valve openings.
volatile byte valveOpen = 0;		// Flag to signal whether purge valve is open or not.
volatile int valveOpenCounter = 0;	// Timer 1, to check how long the purge valve has been open for.
volatile int BMS_addr_arr[9] = {BMS_VOLTAGE_1_ADDR, BMS_VOLTAGE_2_ADDR, BMS_VOLTAGE_3_ADDR, BMS_VOLTAGE_4_ADDR, BMS_VOLTAGE_5_ADDR, BMS_VOLTAGE_6_ADDR, BMS_VOLTAGE_7_ADDR, BMS_VOLTAGE_8_ADDR, BMS_VOLTAGE_9_ADDR};
volatile int flash = 0;
String logFileFilename;

File logFile;
File runNumberFile;
int Address;
byte Data;

// Lookup table for thermistor values
// Array of 1024 temperature values
volatile double thermistorTempLUT[1024] = 
{
125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 125.0, 119.85465031060019, 119.16554299143945, 118.4764356722787, 117.78732835311794, 117.09822103395719, 116.40911371479643, 115.72000639563568, 115.03089907647492, 114.34179175731417, 113.65268443815341, 112.96357711899266, 112.2744697998319, 111.58536248067117, 110.8962551615104, 110.20714784234966, 109.62589672428453, 109.0910025727611, 108.55610842123768, 108.02121426971426, 107.48632011819085, 106.95142596666741, 106.416531815144, 105.88163766362058, 105.34674351209716, 104.81184936057373, 104.27695520905031, 103.7420610575269, 103.20716690600347, 102.67227275448005, 102.13737860295663, 101.60248445143321, 101.0675902999098, 100.53269614838636, 99.99829882588065, 99.58431029609925, 99.17032176631785, 98.75633323653645, 98.34234470675504, 97.92835617697365, 97.51436764719224, 97.10037911741085, 96.68639058762945, 96.27240205784804, 95.85841352806665, 95.44442499828524, 95.03043646850385, 94.61644793872244, 94.20245940894105, 93.78847087915965, 93.37448234937824, 92.96049381959685, 92.54650528981544, 92.13251676003405, 91.71852823025264, 91.30453970047124, 90.89055117068985, 90.47656264090844, 90.06257411112705, 89.72823617026181, 89.40808109878837, 89.08792602731495, 88.76777095584153, 88.44761588436809, 88.12746081289467, 87.80730574142125, 87.48715066994782, 87.16699559847439, 86.84684052700096, 86.52668545552754, 86.2065303840541, 85.88637531258068, 85.56622024110726, 85.24606516963382, 84.9259100981604, 84.60575502668698, 84.28559995521354, 83.96544488374012, 83.6452898122667, 83.32513474079326, 83.00497966931984, 82.68482459784641, 82.36466952637298, 82.04451445489956, 81.72435938342613, 81.40420431195271, 81.08404924047927, 80.76389416900585, 80.44373909753243, 80.12358402605899, 79.84618659903212, 79.59567086169847, 79.34515512436481, 79.09463938703115, 78.84412364969751, 78.59360791236385, 78.3430921750302, 78.09257643769655, 77.8420607003629, 77.59154496302924, 77.34102922569558, 77.09051348836194, 76.83999775102828, 76.58948201369462, 76.33896627636096, 76.08845053902732, 75.83793480169366, 75.58741906436, 75.33690332702635, 75.0863875896927, 74.83587185235905, 74.58535611502539, 74.33484037769173, 74.08432464035809, 73.83380890302443, 73.58329316569078, 73.33277742835713, 73.08226169102348, 72.83174595368982, 72.58123021635616, 72.3307144790225, 72.08019874168886, 71.8296830043552, 71.57916726702155, 71.3286515296879, 71.07813579235425, 70.82762005502059, 70.57710431768693, 70.32658858035327, 70.07607284301963, 69.86257441318743, 69.66521886973442, 69.4678633262814, 69.27050778282837, 69.07315223937536, 68.87579669592235, 68.67844115246933, 68.48108560901632, 68.2837300655633, 68.08637452211029, 67.88901897865728, 67.69166343520426, 67.49430789175125, 67.29695234829822, 67.0995968048452, 66.90224126139219, 66.70488571793918, 66.50753017448616, 66.31017463103315, 66.11281908758014, 65.91546354412712, 65.7181080006741, 65.52075245722109, 65.32339691376808, 65.12604137031505, 64.92868582686204, 64.73133028340902, 64.53397473995601, 64.336619196503, 64.13926365304998, 63.941908109596966, 63.74455256614395, 63.54719702269093, 63.34984147923792, 63.1524859357849, 62.95513039233189, 62.75777484887887, 62.56041930542585, 62.36306376197284, 62.165708218519825, 61.96835267506681, 61.7709971316138, 61.573641588160775, 61.37628604470776, 61.17893050125475, 60.98157495780173, 60.78421941434871, 60.5868638708957, 60.389508327442684, 60.19215278398967, 59.99582860649937, 59.837595723147345, 59.67936283979532, 59.521129956443296, 59.36289707309127, 59.20466418973924, 59.04643130638721, 58.888198423035185, 58.72996553968316, 58.571732656331136, 58.41349977297911, 58.25526688962708, 58.09703400627505, 57.938801122923024, 57.780568239571, 57.62233535621897, 57.46410247286695, 57.30586958951492, 57.14763670616289, 56.989403822810864, 56.831170939458836, 56.67293805610681, 56.51470517275479, 56.35647228940276, 56.19823940605073, 56.040006522698704, 55.881773639346676, 55.72354075599465, 55.56530787264262, 55.4070749892906, 55.24884210593857, 55.090609222586544, 54.932376339234516, 54.77414345588249, 54.61591057253046, 54.45767768917844, 54.29944480582641, 54.14121192247438, 53.982979039122355, 53.82474615577033, 53.6665132724183, 53.50828038906627, 53.350047505714244, 53.19181462236222, 53.033581739010195, 52.87534885565817, 52.71711597230614, 52.55888308895411, 52.40065020560209, 52.24241732225006, 52.084184438898035, 51.92595155554601, 51.76771867219398, 51.60948578884195, 51.45125290548992, 51.293020022137895, 51.134787138785875, 50.97655425543385, 50.81832137208182, 50.66008848872979, 50.50185560537777, 50.34362272202574, 50.185389838673714, 50.027156955321686, 49.89125817620503, 49.75998668662952, 49.628715197054, 49.49744370747849, 49.36617221790297, 49.23490072832746, 49.10362923875194, 48.97235774917643, 48.841086259600914, 48.7098147700254, 48.578543280449885, 48.447271790874375, 48.31600030129886, 48.184728811723346, 48.05345732214783, 47.92218583257232, 47.7909143429968, 47.65964285342129, 47.52837136384577, 47.39709987427026, 47.26582838469474, 47.13455689511923, 47.003285405543714, 46.8720139159682, 46.740742426392686, 46.609470936817175, 46.47819944724166, 46.346927957666146, 46.21565646809063, 46.08438497851512, 45.9531134889396, 45.82184199936409, 45.69057050978857, 45.55929902021306, 45.42802753063754, 45.29675604106203, 45.165484551486514, 45.034213061911004, 44.902941572335486, 44.771670082759975, 44.64039859318446, 44.50912710360895, 44.37785561403343, 44.24658412445791, 44.1153126348824, 43.98404114530689, 43.85276965573137, 43.721498166155854, 43.59022667658034, 43.45895518700483, 43.327683697429315, 43.1964122078538, 43.065140718278286, 42.93386922870277, 42.80259773912726, 42.67132624955174, 42.54005475997623, 42.40878327040071, 42.2775117808252, 42.14624029124968, 42.01496880167417, 41.883697312098654, 41.75242582252314, 41.621154332947626, 41.489882843372115, 41.3586113537966, 41.227339864221086, 41.09606837464557, 40.96479688507006, 40.83352539549454, 40.70225390591903, 40.57098241634351, 40.439710926768, 40.30843943719248, 40.177167947616965, 40.045896458041454, 39.92619337944003, 39.81270932154452, 39.69922526364901, 39.5857412057535, 39.47225714785799, 39.35877308996248, 39.24528903206697, 39.13180497417146, 39.018320916275954, 38.90483685838044, 38.79135280048493, 38.677868742589425, 38.56438468469391, 38.4509006267984, 38.337416568902896, 38.22393251100738, 38.11044845311187, 37.99696439521637, 37.883480337320854, 37.76999627942534, 37.65651222152984, 37.543028163634325, 37.42954410573881, 37.31606004784331, 37.202575989947796, 37.089091932052284, 36.97560787415677, 36.86212381626127, 36.748639758365755, 36.63515570047024, 36.52167164257474, 36.408187584679226, 36.294703526783714, 36.18121946888821, 36.0677354109927, 35.954251353097185, 35.84076729520168, 35.72728323730617, 35.613799179410655, 35.50031512151515, 35.38683106361964, 35.273347005724126, 35.15986294782862, 35.04637888993311, 34.9328948320376, 34.819410774142085, 34.70592671624658, 34.59244265835107, 34.47895860045556, 34.36547454256005, 34.25199048466454, 34.13850642676903, 34.02502236887352, 33.91153831097801, 33.7980542530825, 33.68457019518699, 33.57108613729148, 33.45760207939597, 33.344118021500464, 33.23063396360495, 33.11714990570944, 33.003665847813934, 32.89018178991842, 32.77669773202291, 32.663213674127405, 32.54972961623189, 32.43624555833638, 32.322761500440876, 32.209277442545364, 32.09579338464985, 31.982309326754347, 31.868825268858835, 31.755341210963323, 31.641857153067814, 31.528373095172306, 31.414889037276794, 31.30140497938129, 31.187920921485777, 31.074436863590265, 30.960952805694756, 30.847468747799248, 30.733984689903735, 30.620500632008227, 30.50701657411272, 30.393532516217206, 30.280048458321698, 30.16656440042619, 30.053080342530677, 29.94478190370487, 29.84104037722777, 29.73729885075067, 29.633557324273568, 29.529815797796463, 29.42607427131936, 29.32233274484226, 29.218591218365155, 29.114849691888054, 29.011108165410953, 28.907366638933848, 28.803625112456746, 28.699883585979645, 28.59614205950254, 28.49240053302544, 28.388659006548338, 28.284917480071236, 28.18117595359413, 28.07743442711703, 27.97369290063993, 27.869951374162824, 27.766209847685722, 27.66246832120862, 27.55872679473152, 27.454985268254415, 27.351243741777314, 27.247502215300212, 27.143760688823107, 27.040019162346006, 26.936277635868905, 26.832536109391803, 26.7287945829147, 26.625053056437597, 26.521311529960496, 26.41757000348339, 26.31382847700629, 26.21008695052919, 26.106345424052083, 26.002603897574982, 25.89886237109788, 25.795120844620776, 25.691379318143674, 25.587637791666573, 25.483896265189472, 25.38015473871237, 25.276413212235266, 25.172671685758164, 25.06893015928106, 24.965188632803958, 24.861447106326857, 24.757705579849755, 24.65396405337265, 24.55022252689555, 24.446481000418444, 24.342739473941343, 24.23899794746424, 24.13525642098714, 24.03151489451004, 23.927773368032934, 23.824031841555833, 23.72029031507873, 23.616548788601627, 23.512807262124525, 23.409065735647424, 23.305324209170323, 23.201582682693218, 23.097841156216116, 22.99409962973901, 22.89035810326191, 22.78661657678481, 22.682875050307707, 22.579133523830603, 22.4753919973535, 22.371650470876396, 22.267908944399295, 22.164167417922194, 22.060425891445092, 21.95668436496799, 21.85294283849089, 21.749201312013785, 21.645459785536683, 21.54171825905958, 21.437976732582477, 21.334235206105376, 21.230493679628275, 21.12675215315117, 21.02301062667407, 20.919269100196964, 20.815527573719862, 20.71178604724276, 20.60804452076566, 20.504302994288558, 20.400561467811453, 20.29681994133435, 20.193078414857247, 20.089336888380146, 19.985880001313486, 19.88418843449077, 19.78249686766805, 19.680805300845336, 19.57911373402262, 19.4774221671999, 19.375730600377185, 19.274039033554466, 19.17234746673175, 19.070655899909035, 18.968964333086316, 18.8672727662636, 18.765581199440884, 18.663889632618165, 18.56219806579545, 18.460506498972734, 18.358814932150015, 18.2571233653273, 18.15543179850458, 18.053740231681864, 17.95204866485915, 17.85035709803643, 17.748665531213714, 17.646973964390998, 17.54528239756828, 17.443590830745563, 17.341899263922848, 17.24020769710013, 17.138516130277413, 17.036824563454694, 16.935132996631978, 16.833441429809263, 16.731749862986547, 16.630058296163828, 16.528366729341112, 16.426675162518393, 16.324983595695677, 16.22329202887296, 16.121600462050242, 16.019908895227527, 15.91821732840481, 15.816525761582092, 15.714834194759376, 15.613142627936659, 15.511451061113942, 15.409759494291226, 15.308067927468509, 15.206376360645791, 15.104684793823075, 15.002993227000356, 14.90130166017764, 14.799610093354923, 14.697918526532206, 14.59622695970949, 14.494535392886773, 14.392843826064055, 14.29115225924134, 14.189460692418622, 14.087769125595905, 13.98607755877319, 13.884385991950472, 13.782694425127755, 13.681002858305039, 13.579311291482322, 13.477619724659604, 13.375928157836887, 13.27423659101417, 13.172545024191454, 13.070853457368736, 12.969161890546019, 12.867470323723303, 12.765778756900586, 12.664087190077868, 12.562395623255151, 12.460704056432434, 12.359012489609718, 12.257320922787, 12.155629355964285, 12.053937789141568, 11.95224622231885, 11.850554655496135, 11.748863088673417, 11.6471715218507, 11.545479955027982, 11.443788388205267, 11.34209682138255, 11.240405254559832, 11.138713687737114, 11.037022120914399, 10.935330554091681, 10.833638987268964, 10.731947420446247, 10.630255853623531, 10.528564286800814, 10.426872719978096, 10.325181153155379, 10.223489586332663, 10.121798019509946, 10.02010645268723, 9.91390389474721, 9.806589614032344, 9.69927533331748, 9.591961052602617, 9.484646771887753, 9.377332491172888, 9.270018210458025, 9.162703929743161, 9.055389649028298, 8.948075368313432, 8.840761087598569, 8.733446806883705, 8.626132526168842, 8.518818245453978, 8.411503964739115, 8.30418968402425, 8.196875403309386, 8.089561122594523, 7.982246841879658, 7.874932561164794, 7.7676182804499305, 7.660303999735067, 7.552989719020203, 7.445675438305339, 7.338361157590475, 7.231046876875611, 7.123732596160748, 7.0164183154458835, 6.909104034731019, 6.801789754016156, 6.694475473301292, 6.587161192586429, 6.479846911871564, 6.3725326311567, 6.265218350441836, 6.157904069726973, 6.0505897890121085, 5.943275508297244, 5.835961227582381, 5.728646946867516, 5.621332666152653, 5.514018385437788, 5.406704104722925, 5.2993898240080615, 5.192075543293198, 5.084761262578334, 4.977446981863469, 4.870132701148606, 4.762818420433742, 4.655504139718878, 4.5481898590040135, 4.440875578289151, 4.3335612975742865, 4.226247016859422, 4.118932736144558, 4.011618455429695, 3.904304174714831, 3.7969898939999673, 3.689675613285103, 3.5823613325702395, 3.475047051855375, 3.3677327711405116, 3.2604184904256472, 3.1531042097107846, 3.0457899289959203, 2.938475648281056, 2.8311613675661915, 2.723847086851329, 2.6165328061364646, 2.5092185254216, 2.4019042447067367, 2.294589963991873, 2.187275683277009, 2.0799614025621445, 1.972647121847281, 1.8653328411324175, 1.758018560417554, 1.6507042797026887, 1.5433899989878253, 1.4360757182729618, 1.3287614375580983, 1.221447156843233, 1.1141328761283695, 1.006818595413506, 0.8995043146986426, 0.7921900339837791, 0.6848757532689138, 0.5775614725540503, 0.47024719183918684, 0.36293291112432335, 0.2556186304094581, 0.1483043496945946, 0.04099006897973112, -0.07646851398861973, -0.2001965367021763, -0.32392455941573284, -0.4476525821292894, -0.5713806048428459, -0.6951086275564025, -0.8188366502699591, -0.9425646729835157, -1.0662926956970722, -1.1900207184106288, -1.3137487411241855, -1.437476763837742, -1.5612047865512984, -1.684932809264855, -1.8086608319784117, -1.9323888546919683, -2.0561168774055245, -2.179844900119081, -2.303572922832638, -2.4273009455461945, -2.551028968259751, -2.6747569909733078, -2.798485013686864, -2.9222130364004206, -3.0459410591139773, -3.1696690818275335, -3.29339710454109, -3.4171251272546472, -3.5408531499682034, -3.66458117268176, -3.7883091953953167, -3.912037218108873, -4.03576524082243, -4.159493263535986, -4.2832212862495425, -4.4069493089630996, -4.530677331676656, -4.654405354390213, -4.778133377103769, -4.901861399817325, -5.0255894225308815, -5.149317445244439, -5.273045467957996, -5.396773490671551, -5.520501513385109, -5.644229536098665, -5.767957558812221, -5.8916855815257785, -6.015413604239335, -6.139141626952891, -6.262869649666448, -6.386597672380004, -6.51032569509356, -6.634053717807118, -6.757781740520674, -6.881509763234231, -7.005237785947788, -7.128965808661344, -7.2526938313749, -7.376421854088457, -7.500149876802014, -7.62387789951557, -7.747605922229127, -7.871333944942683, -7.995061967656239, -8.118789990369796, -8.242518013083354, -8.366246035796909, -8.489974058510466, -8.613702081224023, -8.737430103937578, -8.861158126651135, -8.984886149364693, -9.108614172078248, -9.232342194791805, -9.356070217505362, -9.47979824021892, -9.603526262932474, -9.727254285646032, -9.850982308359589, -9.974710331073144, -10.123527262198856, -10.2787897520227, -10.434052241846542, -10.589314731670386, -10.744577221494229, -10.899839711318071, -11.055102201141915, -11.210364690965758, -11.365627180789602, -11.520889670613444, -11.676152160437287, -11.83141465026113, -11.986677140084973, -12.141939629908816, -12.29720211973266, -12.452464609556502, -12.607727099380345, -12.762989589204189, -12.918252079028031, -13.073514568851873, -13.228777058675718, -13.38403954849956, -13.539302038323404, -13.694564528147247, -13.84982701797109, -14.005089507794933, -14.160351997618775, -14.31561448744262, -14.470876977266462, -14.626139467090304, -14.781401956914149, -14.936664446737993, -15.091926936561835, -15.247189426385678, -15.402451916209522, -15.557714406033364, -15.712976895857206, -15.86823938568105, -16.02350187550489, -16.178764365328735, -16.33402685515258, -16.48928934497642, -16.644551834800264, -16.79981432462411, -16.95507681444795, -17.110339304271793, -17.265601794095637, -17.42086428391948, -17.576126773743326, -17.731389263567166, -17.88665175339101, -18.041914243214855, -18.197176733038695, -18.35243922286254, -18.507701712686384, -18.662964202510224, -18.81822669233407, -18.973489182157913, -19.128751671981753, -19.284014161805597, -19.43927665162944, -19.594539141453282, -19.749801631277126, -19.90506412110097, -20.08320071936897, -20.29733426087459, -20.511467802380206, -20.72560134388582, -20.939734885391438, -21.153868426897056, -21.368001968402673, -21.582135509908287, -21.796269051413905, -22.010402592919522, -22.224536134425136, -22.438669675930754, -22.65280321743637, -22.86693675894199, -23.081070300447603, -23.29520384195322, -23.50933738345884, -23.723470924964452, -23.93760446647007, -24.151738007975688, -24.365871549481305, -24.580005090986923, -24.794138632492537, -25.008272173998154, -25.22240571550377, -25.436539257009386, -25.650672798515004, -25.86480634002062, -26.078939881526235, -26.293073423031853, -26.50720696453747, -26.721340506043084, -26.935474047548702, -27.14960758905432, -27.363741130559937, -27.57787467206555, -27.79200821357117, -28.006141755076786, -28.2202752965824, -28.434408838088018, -28.648542379593636, -28.862675921099253, -29.076809462604867, -29.290943004110485, -29.5050765456161, -29.719210087121716, -29.933343628627334, -30.215262335229248, -30.527818415291357, -30.840374495353462, -31.15293057541557, -31.46548665547768, -31.77804273553979, -32.0905988156019, -32.403154895664, -32.71571097572611, -33.02826705578822, -33.34082313585033, -33.65337921591244, -33.965935295974546, -34.278491376036655, -34.591047456098764, -34.90360353616087, -35.21615961622298, -35.52871569628509, -35.8412717763472, -36.15382785640931, -36.466383936471416, -36.778940016533525, -37.091496096595634, -37.40405217665774, -37.71660825671985, -38.02916433678196, -38.34172041684407, -38.65427649690618, -38.96683257696829, -39.279388657030395, -39.591944737092504, -39.90450081715461, -40.33739380666807, -40.823231792014894, -41.30906977736173, -41.794907762708554, -42.28074574805539, -42.766583733402214, -43.25242171874905, -43.738259704095874, -44.2240976894427, -44.709935674789534, -45.19577366013636, -45.681611645483194, -46.16744963083002, -46.653287616176854, -47.13912560152368, -47.624963586870514, -48.11080157221734, -48.596639557564174, -49.082477542911, -49.56831552825783, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0};

void setup(){
	// Initialise serial connections
	Serial.begin(9600);		// Serial reserved for USB connection.
	Serial1.begin(9600);	// Serial1 on pins Rx:19, Tx:18.	
	
	// Initialise pins
    pinMode(PURGE_PIN, OUTPUT);
 	pinMode(FAN_CONTROL_PIN, OUTPUT);
	
	// Start fans at standard fanspeed
	//analogWrite(FAN_CONTROL_PIN, FUEL_CELL_STANDARD_FANSPEED);
		
	//*** INTERRUPT TIMER FOR PURGE VALVE *** //
    // Uncomment to re-enable purge valve control
	/*
	// Initialise timer for purge valve interrupts
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
	
    // set compare match register to desired timer count:
    // Arduino Mega 2560 has 16MHz clock
	// Divide by 1024 to get 15625 Hz.
	// Desired resolution of purge valve is 0.5s => 7812 counts until interrupt.
	OCR1A = 7812;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler, 16MHz clock -> 15.625kHz increments.:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    // enable global interrupts:
    sei();
	*/
	
	// Join I2C bus with no address
	//Wire.begin();
	I2c.begin();
	I2c.timeOut(BMS_REQUEST_TIMEOUT);
	
	// Initialise the SD card
	//SD.begin(4);	// Should really do if(!SD.begin(4)){ /*error handling*/}; // but no
	if(!SD.begin(4)){
		Serial.println("ERROR INITIALISING SD CARD");
	}
	else{
		Serial.println("SD CARD INITIALISED");
	}
	int runNumber = 0;
	// Read the file storing the current lognumber.
	if(!SD.exists("runNum.dat")){
		runNumberFile = SD.open("runNum.dat", FILE_WRITE);
		if(runNumberFile){
			int startFileVal = 0;
			runNumberFile.write(startFileVal);	// Write as an int, when read it must be cast as int.
		}
		runNumberFile.close();
	}
	else{
		runNumberFile = SD.open("runNum.dat", FILE_READ);
		if(runNumberFile){
			runNumber = int(runNumberFile.read());
		}
		else{
			runNumber = 150;	// Some stupid error handling of stupidity
		}
		runNumberFile.close();
		SD.remove("runNum.dat");
		runNumberFile = SD.open("runNum.dat", FILE_WRITE);
		if(runNumberFile){
			runNumberFile.write(++runNumber);
		}
		runNumberFile.close();
	}
	
	// Open a logfile with the runNumber appended to the name, blindly
	logFileFilename = "LF_" + String(runNumber) + ".csv";

	if(!SD.exists(logFileFilename)){	// If the logfile doesn't already exist, just create and open it.
		logFile = SD.open(logFileFilename, FILE_WRITE);
	}
	else{			// If the logfile does already exist, wipe it and make a new one (avoids CSV formatting issues)
		SD.remove(logFileFilename);
		logFile = SD.open(logFileFilename, FILE_WRITE);
	}	
	Serial.print("OPENED LOGFILE: ");
	Serial.println(logFileFilename);
	logFile.flush();

	// Initialise HUD communication
	CommsSetup();
	
	// Set all fields to 0 as default
	
	for(int i = 0; i < 5; i++){
		CommsSetValue(F_VOLTAGE + i, 0.0);	// Values must all be floats
	}
	CommsSetValue(F_SPEED, 0.0);
	CommsSetValue(F_GPSX, 0.0);
	CommsSetValue(F_GPSY, 0.0);
	CommsSetValue(F_IDEALSPEED, 0.0);
	
	HUDUpdate();
	
	// Wait 5 seconds for everything to settle
	delay(500);
	logFile.println("time, BMS_V_1, BMS_V_2, BMS_V_3, BMS_V_4, BMS_V_5, BMS_V_6, BMS_V_7, BMS_V_8, BMS_V_9, heading, speed, latitude, longitude, throttle_voltage");

}
 
void loop(){
	double temperatureArr[6] = {0, 0, 0, 0, 0, 0};	// contains: [therm 1-5 in C, ave temp]
	double BMSVoltageArr[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};	// BMS voltage sensors 1-10
	String logFileLine = String(millis()) + ", ";	// Build a string to avoid repeated SD card operations.
	//String GPSDataArr[5] = {"--", "--", "--", "--", "--"};
	float GPSDataArr[2] = {0.0, 0.0};	// Heading, Speed
	float GPSLatLongArr[2] = {0.0, 0.0};
	String GPSString = "--";
	//String GPSLatLongArr[3] = {"--", "--", "--"};	// Third value is a complete string
	double BMSVoltageTotal = 0;
	double throttleValue = 0;




	logFile.close();	// Close the file from the previous loop then open it again (duct tape code == best code)
	
	logFile = SD.open(logFileFilename, FILE_WRITE);
	// Poll temperature sensors and update fan speed.
	//pollThermistors(temperatureArr);
	//setFanSpeed(temperatureArr);
	
	//Poll battery voltage sensors over I2C
	pollBMSVoltages(BMSVoltageArr);
	for(int i = 0; i < 9; i++){
		logFileLine += String(BMSVoltageArr[i], 4);
		logFileLine += ", ";
		BMSVoltageTotal += BMSVoltageArr[i];
	}
	
	// Poll GPS sensor for position, heading and speed.
	pollGPS(GPSDataArr, GPSLatLongArr, GPSString);

	//logFileLine += GPSLatLongArr[2];
	logFileLine += GPSString;
	logFileLine += ", ";
	
	// Poll throttle
	pollThrottle(throttleValue);
	logFileLine += throttleValue;

	logFile.println(logFileLine);				// Write logfile line to SD card.
	Serial.println(logFileLine);				// Print to serial port/

	for(int i = 0; i < 5; i++){
		CommsSetValue(F_VOLTAGE + 0, float(BMSVoltageTotal));
	}
	CommsSetValue(F_SPEED, GPSDataArr[1]);	

	// HUD DEMO CODE:
	//hudDemo();
	
	/*
	flash = (flash + 1) % 384;
	flash++;
	float pos = flash/384.0;	
	
	CommsSetValue(F_GPSX, 5 + sin(2*PI * pos)*4);
	CommsSetValue(F_GPSY, 5 + cos(2*PI * pos)*4);
	CommsSetValue(F_VOLTAGE + 0, 0.5 + sin(2*PI*pos + 0.2));
	CommsSetValue(F_VOLTAGE + 1, 0.5 + sin(2*PI*pos + 0.4));
	CommsSetValue(F_VOLTAGE + 2, 0.5 + sin(2*PI*pos + 0.8));
	CommsSetValue(F_VOLTAGE + 3, 0.5 + sin(2*PI*pos + 1.0));
	CommsSetValue(F_VOLTAGE + 4, 0.5 + sin(2*PI*pos + 1.2));
	CommsSetValue(F_SPEED, 25 + sin(2*PI*pos)*8);	
	HUDUpdate();
*/	
	
	HUDUpdate();
}	

void hudDemo(){
	flash = (flash + 1) % 384;
	flash++;
	float pos = flash/384.0;	
	
	CommsSetValue(F_GPSX, 5 + sin(2*PI * pos)*4);
	CommsSetValue(F_GPSY, 5 + cos(2*PI * pos)*4);
	CommsSetValue(F_VOLTAGE + 0, 0.5 + sin(2*PI*pos + 0.2));
	CommsSetValue(F_VOLTAGE + 1, 0.5 + sin(2*PI*pos + 0.4));
	CommsSetValue(F_VOLTAGE + 2, 0.5 + sin(2*PI*pos + 0.8));
	CommsSetValue(F_VOLTAGE + 3, 0.5 + sin(2*PI*pos + 1.0));
	CommsSetValue(F_VOLTAGE + 4, 0.5 + sin(2*PI*pos + 1.2));
	CommsSetValue(F_SPEED, 25 + sin(2*PI*pos)*8);	
	HUDUpdate();
}

void pollThrottle(double& throttleVal){
	double VPerDiv = 5.0/1024.0;
	throttleVal = analogRead(THROTTLE_SENSE_PIN) * VPerDiv;
	return;
}

void pollGPS(float GPSDataArr[2], float GPSLatLongArr[3], String& GPSString){		// Taken from example code provided for the GPS Shield	
	String tempString;
	String tempString1;
	String tempMins;	// Used for conversion to decimal degrees
	String tempDegs;
	// Heading
	Address = 44;                                       // Point to Heading 1 register
	Data = GetDouble();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString = String(Data, DEC);
	tempString1 = String(Data, DEC);
	Address = 46;                                       // Point to Heading 2 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(".");
	tempString += String(Data, DEC) + ".";
	tempString1 += String(Data, DEC) + ".";
	Address = 47;                                       // Point to Heading 3 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.println(Data, DEC);                          // and print to PC
	tempString += String(Data, DEC) + ", ";
	
	GPSDataArr[0] = tempString1.toFloat();

	//Serial.print("Speed: ");
	Address = 52;                                       // Point to Speed 1 register
	Data = GetDouble();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);	
	tempString1 = String(Data, DEC);	
	Address = 54;                                       // Point to Speed 2 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(".");
	tempString += String(Data, DEC) + ".";
	tempString1 += String(Data, DEC) + ".";
	Address = 55;                                       // Point to Speed 3 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.println(Data, DEC);                          // and print to PC
	tempString += String(Data, DEC) + ", ";

	GPSDataArr[1] = tempString1.toFloat();

	//Serial.print("Latitide: ");
	Address = 14;                                       // Point to Latitude 1 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 = String(Data, DEC);
	tempDegs = String(Data, DEC);						// Should contain the degrees latitude
	Address = 15;                                       // Point to Latitude 2 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(" ");
	tempString += String(Data, DEC) + " ";
	tempString1 += String(Data, DEC) + " ";
	tempDegs += String(Data, DEC);

	Address = 16;                                       // Point to Latitude 3 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins = String(Data, DEC);
	Address = 17;                                       // Point to Latitude 4 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(".");
	tempString += String(Data, DEC) + ".";
	tempString1 += String(Data, DEC) + ".";
	tempMins += String(Data, DEC) + ".";
	Address = 18;                                       // Point to Latitude 5 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 19;                                       // Point to Latitude 6 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 20;                                       // Point to Latitude 7 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 21;                                       // Point to Latitude 8 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 22;                                       // Point to Latitude character register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.println(Data);                               // and print to PC
	tempString += String(Data, DEC) + ", ";
	tempMins += String(Data, DEC);

	GPSLatLongArr[0] = (tempDegs.toFloat() + (tempMins.toFloat()/60));

	//Serial.print("Longitude: ");
	Address = 23;                                       // Point to Longitude 1 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 = String(Data, DEC);
	tempDegs = String(Data, DEC);
	Address = 24;                                       // Point to Longitude 2 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempDegs += String(Data, DEC);
	Address = 25;                                       // Point to Longitude 3 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(" ");
	tempString += String(Data, DEC) + " ";
	tempString1 += String(Data, DEC) + " ";
	tempDegs += String(Data, DEC);
	Address = 26;                                       // Point to Longitude 4 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins = String(Data, DEC);
	Address = 27;                                       // Point to Longitude 5 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	//Serial.print(".");
	tempString += String(Data, DEC) + ".";
	tempString1 += String(Data, DEC) + ".";
	tempMins += String(Data, DEC) + ".";
	Address = 28;                                       // Point to Longitude 6 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 29;                                       // Point to Longitude 7 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 30;                                       // Point to Longitude 8 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 31;                                       // Point to Longitude 9 register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.print(Data, DEC);                            // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	Address = 32;                                       // Point to Longitude character register
	Data = GetSingle();                                 // Read registers from GPM
	//Serial.println(Data);                               // and print to PC
	tempString += String(Data, DEC);
	tempString1 += String(Data, DEC);
	tempMins += String(Data, DEC);
	
	GPSLatLongArr[1] = (tempDegs.toFloat() + (tempMins.toFloat()/60));
	GPSString = tempString;	
	//GPSLatLongArr[1] = tempString1;
	//GPSLatLongArr[2] = tempString;
	return;
}

// GetDouble is a service routine for pollGPS.
int GetDouble(){                      // Get double register value from GPM
	int Value = 0; 
	byte H_Byte = 0;
	byte L_Byte = 0;
	int startTime = 0;

	I2c.read(GPS_SHIELD_ADDR, Address, 2);
	bool failedFlag = false;
	startTime = millis();
	while(I2c.available() < 2){			// Wait until 2 bytes available, use a timeout to prevent hanging
		if((millis() - startTime) >= BMS_REQUEST_TIMEOUT){	// Use the BMS timeout because reasons
			failedFlag = true;
			break;
		}
	}
	if(!failedFlag){
		H_Byte = I2c.receive();
		L_Byte = I2c.receive();
	}
	else{	// Error handling
		H_Byte = 0;
		L_Byte = 0;
	}

	Value = (H_Byte * 10) + L_Byte;                     // Adjust for one byte
	return(Value);                              
}

// GetSingle is a service routine for pollGPS.
int GetSingle(){                      // Get single register value from GPM
	int Value = 0; 
	int startTime = 0;
	
	I2c.read(GPS_SHIELD_ADDR, Address, 1);
	bool failedFlag = false;
	startTime = millis();
	while(I2c.available() < 1){		
		if((millis() - startTime) >= BMS_REQUEST_TIMEOUT){	// Use the BMS timeout because reasons
			failedFlag = true;
			break;
		}
	}
	if(!failedFlag){
		Value = I2c.receive();
	}
	else{	// Error handling
		Value = 0;
	}
	return(Value);                              
}

void pollBMSVoltages(double BMSVoltageArr[9]){
	byte i2cByte1 = 0;
	byte i2cByte2 = 0;
	double vPerDiv = 5.0/1024.0;
	int startTime = 0;
	bool failedFlag = false;

	for(int i = 0; i < 9; i++){
		failedFlag = false;
		I2c.read(BMS_addr_arr[i], 2);	// Request 2 bytes from the BMS voltage board
		startTime = millis();
		while(I2c.available() < 2){			// Wait until 2 bytes available, use a timeout to prevent hanging
			if((millis() - startTime) >= BMS_REQUEST_TIMEOUT){
				failedFlag = true;
				break;
			}
		}
		if(!failedFlag){
			i2cByte1 = I2c.receive();
			i2cByte2 = I2c.receive();
			BMSVoltageArr[i] = (i2cByte1 + (256 * i2cByte2)) * vPerDiv;
			if(BMSVoltageArr[i] < 0){
				BMSVoltageArr[i] = 0;
			}	// Clamp bizarre -159V error to 0 (happens when input floats)
		}
		else{
			BMSVoltageArr[i] = 0.0;
		}
	}
	return;
}

void pollThermistors(double thermistorArr[6]){	// Polls thermistors, converts voltage to a temperature, returns temps and mean temp.
	// Read the thermistors.
	int thermistorArrTMP[5] = {0, 0, 0, 0, 0};	// Stores the raw 0-1023 value returned by analogRead(PIN).

	thermistorArrTMP[0] = analogRead(THERMISTOR_IN_1);	// Maps to 0-1023 for 0V-5V. 0.004883V/increment.
	thermistorArrTMP[1] = analogRead(THERMISTOR_IN_2);	
	thermistorArrTMP[2] = analogRead(THERMISTOR_IN_3);	
	thermistorArrTMP[3] = analogRead(THERMISTOR_IN_4);	
	thermistorArrTMP[4] = analogRead(THERMISTOR_IN_5);	
	
	// LUT to get temps for each.
	thermistorArr[0] = thermistorTempLUT[thermistorArrTMP[0]];
	thermistorArr[1] = thermistorTempLUT[thermistorArrTMP[1]];
	thermistorArr[2] = thermistorTempLUT[thermistorArrTMP[2]];
	thermistorArr[3] = thermistorTempLUT[thermistorArrTMP[3]];
	thermistorArr[4] = thermistorTempLUT[thermistorArrTMP[4]];
	
	// Compute average
	double totalTmp = 0;
	for(int i = 0; i < 6; i++){
		totalTmp += thermistorArr[i];
	}
	thermistorArr[5] = totalTmp/6;
	return;
}

void setFanSpeed(double temperatureArr[6]){	// Linear controller for fuel cell cooling fans
	int tDiff = temperatureArr[5] - FUEL_CELL_IDEAL_TEMP;	// > ideal is +ve, < ideal is -ve.
	float fanSpeed = FUEL_CELL_STANDARD_FANSPEED + (tDiff * FUEL_CELL_FAN_CONTROL_SENSITIVITY);
	
	// Clamp fanSpeed
	if(fanSpeed < FUEL_CELL_MIN_FANSPEED){
		fanSpeed = FUEL_CELL_MIN_FANSPEED;
	}
	if(fanSpeed >= 255){
		fanSpeed = 255;
	}
	
	// Check if panic fullspeed on individuals or mean temp.
	for(int i = 0; i < 6; i++){
		if(temperatureArr[i] >= FUEL_CELL_INDIVIDUAL_MAX_TEMP){
			fanSpeed = 255;
		}
	}
	analogWrite(FAN_CONTROL_PIN, fanSpeed);
}

ISR(TIMER1_COMPA_vect){ // ISR handles purge valve opening/closing.
	if(valveOpen == 1){
		valveOpenCounter++;
		if(valveOpenCounter >= (2 * PURGE_OPEN_SECONDS)){
			Serial.println("CLOSE VALVE");
			digitalWrite(PURGE_PIN, LOW);
			valveOpenCounter = 0;		// Valve open timer reset to 0.
			valveOpen = 0;				// Valve status reset to closed.
		}
	}
	else{
		time1Counter++;
		if(time1Counter >= (2 * PURGE_CLOSED_SECONDS)){
			Serial.println("OPEN VALVE");
    		digitalWrite(PURGE_PIN, HIGH);
			time1Counter = 0;			// Valve closed timer reset to 0.
			valveOpen = 1;				// Valve status set to open.
		}
	}
}



