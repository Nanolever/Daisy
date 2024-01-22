#include "PERSISTENCE.hpp"

void SPIFFS::initializeSPIFFS(void){
    ret = esp_vfs_spiffs_register(&(this->conf));
    if (ret != ESP_OK) { //Controllo corretta registrazione e montaggio
        if (ret == ESP_FAIL) printf("Failed to mount or format filesystem\n");
        else if (ret == ESP_ERR_NOT_FOUND) printf("Failed to find SPIFFS partition\n");
        else printf("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
	else {	//Controllo integrità
		ret = esp_spiffs_check(conf.partition_label);
        if (ret != ESP_OK) printf("SPIFFS_check() failed (%s)\n", esp_err_to_name(ret));
        else printf("SPIFFS initialized successfully\n");
    }
}

SPIFFS::SPIFFS(void){
    this->initializeSPIFFS();
}

bool SPIFFS::saveCalfact(float calfact){
    FILE *f = fopen("/spiffs/calfact.bin", "wb");
    if(f==NULL) return 1;
    fwrite(&calfact, sizeof(calfact), 1, f);
    fclose(f);
    return 0;
}

bool SPIFFS::saveOffset(long offset){
    FILE *f = fopen("/spiffs/offset.bin", "wb");
    if(f==NULL) return 1;
    fwrite(&offset, sizeof(offset), 1, f);
    fclose(f);
    return 0;
}

float SPIFFS::retrieveCalfact(void){
    printf("Retrieving calfact\n");
    float calfact;
    FILE *f = fopen("/spiffs/calfact.bin", "rb");
    if(f==NULL) {
        printf("Could not find Calib data\n");
        return 1;
    }
    fread(&calfact, sizeof(calfact), 1, f);
    fclose(f);
    printf("Correctly retrieved calfact: %f\n", calfact);
    return calfact;
}

long SPIFFS::retrieveOffset(void){
    printf("Retrieving offset\n");
    long offset;
    FILE *f = fopen("/spiffs/offset.bin", "rb");
    if(f==NULL) {
        printf("Could not find Offset data\n");
        return 0;
    }
    fread(&offset, sizeof(offset), 1, f);
    fclose(f);
    printf("Correctly retrieved offset: %ld\n", offset);
    return offset;
}
