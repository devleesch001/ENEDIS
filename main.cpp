#include "header.h"

namespace {
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

int fd=0, n;
bool stop;
string trame, ADSC, EAST, IRMS1, URMS1, PRM;

int  main(int argc, char** argv)
{
    Config config;
    try
    {
        /** Define and parse the program options */
        namespace po = boost::program_options;
        po::options_description desc("Options");
        desc.add_options()
                ("help,h", "Print help messages")
                ("config,c", "show config")
                ("config-re,r", "reset config to default")
                ("version,v", "version");
        po::variables_map vm;
        try{
            po::store(po::parse_command_line(argc, argv, desc),
                      vm); // can throw

            /** --help option */
            if (vm.count("help")){
                std::cout << "Basic Command Line Parameter App" << std::endl
                          << desc << std::endl;
                return SUCCESS;
            }
            if (vm.count("config")){
                displayConfig(config);
                return SUCCESS;
            }
            if (vm.count("config-re")){
                reloadConfig();
                return SUCCESS;
            }
            if (vm.count("version")){
                std::cout << "version" << std::endl;
                return SUCCESS;
            }

            po::notify(vm); // throws on error, so do after help in case
            // there are any problems
        }
        catch(po::error& e){
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return ERROR_IN_COMMAND_LINE;
        }
    }
    catch(std::exception& e){
        std::cerr << "Unhandled Exception reached the top of main: "
                  << e.what() << ", application will now exit" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;


    }

    openport();
    thread serialread (readport);
    serialread.detach();

    while(1){
        postgres(ADSC, PRM, EAST, IRMS1, URMS1);
        sleep(1);
        if (stop == true){
            return 0;
        }
    }
}

struct termios enedis;

void openport(void){
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY |O_NDELAY );
    if (fd < 0){
        perror(MODEMDEVICE);
    }

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd,&enedis); /* save current serial port settings */
    bzero(&enedis, sizeof(enedis));

    enedis.c_cflag = BAUDRATE | CRTSCTS | CS7 | CSTOPB;

    enedis.c_iflag = IGNPAR | ICRNL;

    enedis.c_oflag = 0;
    enedis.c_lflag = ICANON;
    enedis.c_cc[VINTR]    = 0;     /* Ctrl-c */
    enedis.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
    enedis.c_cc[VERASE]   = 0;     /* del */
    enedis.c_cc[VKILL]    = 0;     /* @ */
    // enedis.c_cc[VEOF]     = 4;     /* Ctrl-d */
    enedis.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    enedis.c_cc[VMIN]     = 0;     /* blocking read until 1 character arrives */
    enedis.c_cc[VSWTC]    = 0;     /* '\0' */
    enedis.c_cc[VSTART]   = 0;     /* Ctrl-q */
    enedis.c_cc[VSTOP]    = 0;     /* Ctrl-s */
    enedis.c_cc[VSUSP]    = 0;     /* Ctrl-z */
    enedis.c_cc[VEOL]     = 0;     /* '\0' */
    enedis.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    enedis.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    enedis.c_cc[VWERASE]  = 0;     /* Ctrl-w */
    enedis.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
    enedis.c_cc[VEOL2]    = 0;     /* '\0' */
}

void readport(void){
    unsigned char buff;
    while (1) {
        n = read(fd, &buff, 1);
        if (n == 0) break;


        if (buff == LF){
            //printf("%c",buff);
        }

        else if(buff == CR){
            analyse(trame);
            //cout << trame;
            //printf ("%c", buff);
            trame = '\0';
        }
        else if(buff == ETX){
            //printf("\nETX = 0x%x", buff);
        }
        else if(buff == STX){
            //printf("\nSTX = 0x%x", buff);
        }
        else {
            trame += buff;
        }
    }
}

void analyse(string a) {
    istringstream iss(a);
    string etiquette, donnee, checksum;

    getline(iss, etiquette, '\t'); // permet de sépare en 3 string les 3 elemeent transmi separe par une tabulation
    getline(iss, donnee,    '\t');
    getline(iss, checksum);

    etiquette.erase (0,1); //     utilise car le string etiquette possede un char vide a [0]

    if (etiquette == "ADSC"){
        ADSC = donnee;
    }
    if (etiquette == "PRM"){
        PRM = donnee;
    }
    else if(etiquette == "EAST"){
        EAST = donnee;
    }
    else if(etiquette == "IRMS1"){
        IRMS1 = donnee;
    }
    else if(etiquette == "URMS1"){
        URMS1 = donnee;
    }
}

void historique(){
    /*
ostringstream oss;
oss << "ADC0" << " " << donnee << " " << checksum;
historique = oss.str();
cout << historique;
printf("\n");
//socket(historique);

if (etiquette == "IRMS1"){
    //socket(donnee);
    printf("\n");

}
*/
}