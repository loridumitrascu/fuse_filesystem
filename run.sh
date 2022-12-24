#! /bin/bash
if [ $# -eq 0 ]
  then
    echo "Secured Ramdisk FileSystem With FUSE";
    echo "Project by: Catanoiu Simona & Andreea Dumitrascu"
    echo "Welcome!"
    echo "Run again with one of the following arguments:"
    echo "-i - Install dependencies"
    echo "-c - Compile the application"
    echo "-m <mount_point> <disk_image_name> - Mount the filesystem"
    echo "-u <mount_point> - Unmount the filesystem"
    echo "-d <mount_point> <disk_image_name> - Mount in debug mode"
    echo "-x <mount_point> <disk_image_name> - Clean application"
  else
    case $1 in
        -i)
        if [ $# -eq  1 ]
        then
            sudo apt-get update
            sudo apt-get install libfuse-dev
        else
            echo "Invalid command. Format: ./run.sh -i"
        fi
        ;;

        -c)
        if [ $# -eq  1 ]
        then
            make all
        else
            echo "Invalid command. Format: ./run.sh -c"
        fi
            
        ;;

        -d)
        if [ $# -eq  3 ]
        then
            make mount_point=$2 disk_path=$3 debugmount
        else
            echo "Invalid command. Format: ./run.sh -d <mount_point> <disk_image_name>"
        fi
        ;;   

        -m)
        if [ $# -eq  3 ]
        then
            make mount_point=$2 disk_path=$3 mount
        else
            echo "Invalid command. Format: ./run.sh -m <mount_point> <disk_image_name>"
        fi
        ;; 

        -u)
        if [ $# -eq  2 ]
        then
            make mount_point=$2 umount
        else
            echo "Invalid command. Format: ./run.sh -u <mount_point>"
        fi
        ;;    

        -x)
        if [ $# -eq  3 ]
        then
            make mount_point=$2 disk_path=$3 clean
        else
            echo "Invalid command. Format: ./run.sh -c <mount_point> <disk_image_name>"
        fi
        ;;

        *)
        echo "Invalid command." 
        ;;
    esac
fi
