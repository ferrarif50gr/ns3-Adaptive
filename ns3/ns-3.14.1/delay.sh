path2="/media/My\ Passport/EEE/simulations/queue"
queueSize="1 2 5 10 50 100 200"

for j in $path2$queueSize;
do
cd "$j" ;
for i in switched-ethernet-MyScenario2-*;
do 
	less $i |  
	grep -e "^+" -e "^-" | 
	awk '{print $22 " " $1 " " $2 " "$3}' | 
	sort -n | 
	sed -e 's/\/NodeList\///' -e 's/\/DeviceList\/1\/$ns3::SwitchedEthernetNetDevice\/TxQueue\/[a-zA-Z]*//' | 
	awk 'BEGIN{id[1]=-1; arrival[1]=-1; id[2]=-1; arrival[2]=-1;}{if ($4==0) i=1; else i=2; if(id[i]==$1) {a=$3-arrival[i];s[i]+=a; ss[i]+=(a*a); n[i]++}; id[i]=$1; arrival[i]=$3;}END{for(i=1;i<=2;i++) printf("(if/pkts/mean/dev): %d %d %f %f\n", i-1, n[i], s[i]/n[i], sqrt((s[i]*s[i]-ss[i]/n[i])/(n[i]-1)))}'  ;

#	awk 'BEGIN{id[1]=-1; arrival[1]=-1; id[2]=-1; arrival[2]=-1;}{if ($4==0) i=1; 
#		else i=2; if(id[i]==$1){a=$3-arrival[i]; s[i]+=a; ss[i]+=(a*a); n[i]++}; 
#			id[i]=$1; 
#			arrival[i]=$3;
#	}
#	END
#		{
#			for(i=1;i<=2;i++) 
#				printf("%d %d %f %f\n", i-1, n[i], s[i]/n[i], sqrt((s[i]*s[i]-ss[i]/n[i])/(n[i]-1)));
#		}' ;
done
done
#for k in switched-ethernet-MyScenario2-*
#do

#        less $k |
#        grep -e "+ " -e "- " |
#        awk '{print $22 " " $1 " " $2 " "$3}' |
#        sort -n |
#        sed -e 's/\/NodeList\///' -e 's/\/DeviceList\/1\/$ns3::SwitchedEthernetNetDevice\/TxQueue\/[a-zA-Z]*//' |
#        awk '{if ($4=="0") print $0; if(id==$1) print $3-arrival; id=$1; arrival=$3;}' > $k-if0;
#	
#done
#rm *-if1-*
#rm *-if0-*



#less switched-ethernet-MyScenario2-5-0.002-0-0.002-0-10-1-exp.tr | grep -e "^+" -e "^-" | awk '{print $22 " " $1 " " $2 " "$3}' | sort -n | sed -e 's/\/NodeList\///' -e 's/\/DeviceList\/1\/$ns3::SwitchedEthernetNetDevice\/TxQueue\/[a-zA-Z]*//' | awk 'BEGIN{id[1]=-1; arrival[1]=-1; id[2]=-1; arrival[2]=-1;}{if ($4==0) i=1; else i=2; if(id[i]==$1) {a=$3-arrival[i];s[i]+=a; ss[i]+=(a*a); n[i]++}; id[i]=$1; arrival[i]=$3;}END{for(i=1;i<=2;i++) printf("%d %d %f %f\n", i-1, n[i], s[i]/n[i], sqrt((s[i]*s[i]-ss[i]/n[i])/(n[i]-1)))}' 

