#!/bin/bash
 
# by strobelstefan.org
# 2019-10-23
# Version: 2.0
# https://strobelstefan.org/?p=5985
 
#
# This script creates a full clone of your Raspberry Pi´s SD card.
#
 
###################################
# Define Variables
###################################
 
# Storage device as defined in your /etc/fstab.
mountpoint='/media/usb'
 
# Path were the image of your SD card should be saved to
STORAGEPATH="/media/usb"
 
# Location of Nextcloud Installation
NEXTCLOUDINSTALLATION="/var/www/html/nextcloud"
 
# E-Mail Address
EMAIL="mail@email.de"
 
# Image name
IMAGENAME="rpi"
 
#Log File location and name
LOGFILE="/var/log/"$HOSTNAME_${IMAGENAME}_"backup-image.log"
 
 
 
###################################
# This removes your old log file
###################################
# This removes your old log file.
# When you run the script the first time an error will be displayed, 
# because no log file is in the defined path.
rm ${LOGFILE}
 
 
###################################
# MOUNTPOINT Section - Check Mount point Availability
###################################
# It checks if your mount point is accessible by your RPi.
# This is a crucial step, if the storage is not available the clone process of the SD card cannot conducted.
# Process
# 1. Check if mount point is accessible
# 2. If YES go to DELETION Section
# 3.1 If NO, try to mount storage device as defined in /etc/fstab
# 3.2 If mount is again not successful exit script, no further action will be conducted
 
if [ "$(findmnt ${mountpoint})" ] ;
    then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Mount point accessible by your "$HOSTNAME >> ${LOGFILE}
    else
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Mount point was not accessible, try to mount it now as defined in your /etc/fstab" >> ${LOGFILE}
 
    #This command mounts all storages defined in /etc/fstab
    mount -a
 
    if [ $? != 0 ]
        then
            echo $(date +%Y-%m-%d_%H-%M-%S) " - Mount of storage in first try successfully completed" >> ${LOGFILE}
        sleep 5
            mount -a
        if [ $? != 0 ]
        then
            echo $(date +%Y-%m-%d_%H-%M-%S) " - Backup FAILED! Was not able to mount your storage device. Stop backup process. You have to check it manually." >> ${LOGFILE}
            echo "Sent backup status via e-mail" | mutt ${EMAIL} -a ${LOGFILE} -s $HOSTNAME" - Backup FAILED" >> ${LOGFILE}
        exit
        fi
    fi
 
fi
 
 
##################################################################
# DELETION Section - Remove old Images from Storage Device
##################################################################
# Use this command with CAUTION!
# This will help you to automatically remove old images from your storage device to avoid that your
# storage will run out of disk space
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Start to delete files older than defined time " >> ${LOGFILE}
 
 
# Uncomment if the files should be identified by days, file > 30 days than it gets deleted
#find ${STORAGEPATH}/*.* -mtime +30 -exec rm -r {} \;
 
# Uncomment if you would like to use minutes file > 10080 minutes than it gets deleted
find ${STORAGEPATH}/*.* -type f -mmin +43200 -exec rm {} \;
 
if [ $? != 0 ]
    then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Deletion of old image files successfully completed" >> ${LOGFILE}
     if [ $? != 0 ]
     then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Was not able to delete old image files. You have to check it manually." >> ${LOGFILE}
    break
    fi
fi
 
 
###################################
# Nextcloud Maintenance Mode ON
###################################
#Aktiviere den Maintenancemode für Nextcloud - Keine Useranmeldung mehr möglich!
 
#Wechsel in das Nexcloud-Verzeichnis
cd ${NEXTCLOUDINSTALLATION}
 
sudo -u www-data php occ maintenance:mode --on
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Nextcloud maintenance mode ON" >> ${LOGFILE}
 
 
###################################
# CLONE Section - Clone SD Card Image
###################################
# This line creates a full copy of the SD card and writes it as an image file to the defined patch
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Started to clone image" >> ${LOGFILE}
 
# Saves a plain img file on your storage device
sudo dd if=/dev/mmcblk0 of=${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d).img bs=1MB
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Finished to clone image" >> ${LOGFILE}
 
 
###################################
# Nextcloud Maintenance Mode OFF
###################################
#Entferne den Maintenancemode für Nextcloud - Useranmeldungen sind wieder möglich!
 
#Wechsel in das Nexcloud-Verzeichnis
cd ${NEXTCLOUDINSTALLATION}
 
sudo -u www-data php occ maintenance:mode --off
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Nextcloud maintenance mode OFF" >> ${LOGFILE}
 
 
###################################
# Resize dd Image
###################################
# Resize image with pishrink
# Please see https://github.com/Drewsif/PiShrink for further details
# pishrink.sh must be located in the same directory as this script!
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Started to resize image" >> ${LOGFILE}
 
sudo /bin/bash /etc/scripts/pishrink.sh -d ${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d).img ${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d)-small.img
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Finished to resize big image" >> ${LOGFILE}
 
 
#Delete big image file
echo $(date +%Y-%m-%d_%H-%M-%S) " - Started to delete big image" >> ${LOGFILE}
 
sudo rm ${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d).img
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Finished to delete big image" >> ${LOGFILE}
 
 
 
# Creates a compressed file of the resized image
# This command will create a compressed gz archive of the small image file.
# The small file will get deleted during the process if you would like to keep
# the small image file use the command gzip -k ${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d)-small.img
# Before you change the compression process check your disk space size
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Started to compress small image" >> ${LOGFILE}
 
gzip -q ${STORAGEPATH}/${IMAGENAME}_$(date +%Y-%m-%d)-small.img
 
echo $(date +%Y-%m-%d_%H-%M-%S) " - Finished to compress small image" >> ${LOGFILE}
 
 
 
 
if [ $? != 0 ]
    then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Image file created" >> ${LOGFILE}
     if [ $? != 0 ]
    then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Was not able to create your image file. You have to check it manually." >> ${LOGFILE}
    break
    fi
fi
 
 
 
###################################
# UMOUNT Section - Unmount Storage Device
###################################
# This command unmounts the defined storage device.
# In the first try it will gently try to unmount, if the device is busy the command will force the unmount.
 
if [ "$(umount ${mountpoint})" ] ; 
    then
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Umounted your storage device" >> ${LOGFILE}
    else
        echo $(date +%Y-%m-%d_%H-%M-%S) " - Umount of storage device was not possible in first run, wait for 30 seconds" >> ${LOGFILE}
  
    sleep 30
  
    if [ $? != 0 ]
        then
            echo $(date +%Y-%m-%d_%H-%M-%S) " - Umount was successful" >> ${LOGFILE}
        sleep 5
            umount ${mountpoint}
        if [ $? != 0 ]
        then
            echo $(date +%Y-%m-%d_%H-%M-%S) " - Umount successful" >> ${LOGFILE}
        exit
        fi
    fi
  
fi
 
# Skript finished
echo $(date +%Y-%m-%d_%H-%M-%S) " - Mission Accomplished!!! System is going for a reboot." >> ${LOGFILE}
 
# Send status via e-mail
echo "Sent backup status via e-mail" | mutt ${EMAIL} -a ${LOGFILE} pishrink.log -s $HOSTNAME" - Backup SUCCESSFULL" >> ${LOGFILE}
 
#Optional
# If you like to reboot your system, please uncomment
#sudo reboot
