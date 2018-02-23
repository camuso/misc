for i in `objdump -t <modulename> | awk '/UND/ {print $4}'`
do
        for j in i686 ppc64 s390x x86_64
        do
                grep -q $i ~/cvs/rpms/kernel/RHEL-6/Module.kabi_$j
                if [ $? -eq 0 ]
                then
                        echo found symbol $i in Module.kabi_$j
                fi
        done
done

