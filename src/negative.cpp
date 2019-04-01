#include "negative.h"

using namespace std;

namespace NEG{

int NB_THREADS=24;

void triScore(TableTuple &donnees, Space d){
//  Cette procédure trie les données suivant le score (somme des valeurs des attributs)
    DataType i, n=donnees.size();
    Space j;
    vector<DataType> aux(n);
    TableTuple auxT=donnees;
    for (i=0;i<n;i++){
        aux[i]=donnees[i][1];
        for (j=2;j<=d;j++) aux[i]+=donnees[i][j];
    }
    vector<DataType> index(donnees.size());
    for (i = 0; i != (DataType)index.size(); ++i) index[i] = i;
    sortIndexes(aux, index);
    for (i=0;i<n;i++){
        donnees[i]=auxT[index[i]];
    }
}

void visualisation_pairs(vector<USetDualSpace> listUSetDualSpace){

    cout <<"*****visualisation_pairs*****"<<endl;
    for(int i=0; i<listUSetDualSpace.size(); i++){
        cout <<"t"<<i<<": ";
        for (auto it_uset = listUSetDualSpace[i].begin(); it_uset!=listUSetDualSpace[i].end(); it_uset++){
            cout <<it_uset->dom <<" "<<it_uset->equ <<" ; ";
        }
        cout <<endl;
    }

}

bool pet_pair(const DualSpace &sp1, const DualSpace &sp2 ){
    auto n1=sp1.dom + sp1.equ;
    auto n2=sp2.dom + sp2.equ;
    return __builtin_popcount(n1) >__builtin_popcount(n2);
}

inline DualSpace domDualSubspace_1(const Point &t1, const Point &t2, const Space &d){
//  retourne le sous espace dans lequel t1 domine t2//je pense que c'est t2 qui domine t1
//  les sous espaces sont codés en un nombre décimal
//  exemples (codage) quand d=4, ABCD->15, AD->9, A->1, B->2, C->4, BC -> 5
    Space j;
    Space poids1=0, poids2=0;
    DualSpace sortie;
    sortie.dom=0;
    sortie.equ=0;
    long pow=1;
    unsigned int dec=1;
    if (t1[0]==t2[0]) return sortie;
    for(j = 1; j <= d ; ++j){
        if(t1[j] < t2[j]) {sortie.dom+=pow;++poids1;}
        else if(t1[j] == t2[j]) {sortie.equ+=pow;++poids2;}
        pow*=2;
	   //pow = (pow << 1);
    }
    sortie.poids=(1<<(poids1+poids2))-(1<<poids2);
    return sortie;
}


void insertDualSpaceToUSet(DualSpace sp, const Space &d, USetDualSpace &uSetDualSpace, Space &all, Space &maxDom, Space &sizeMaxDom, bool &sortie){
//retourne true si le tuple est completement dominé (ne plus le comparer à d'autres tuples)
    if (sp.dom==all){
        USetDualSpace sp1;
        sp1.insert(sp);
        uSetDualSpace.swap(sp1);
        sortie=true;
        return;
    }else if (sp.dom!=0){
        if (!estInclusDans(sp.dom+sp.equ, maxDom)){
            if (spaceSize(sp.dom)>sizeMaxDom){
                maxDom=sp.dom;
                sizeMaxDom=spaceSize(sp.dom);
            }
            uSetDualSpace.insert(sp);
        }
    }
}





long creationStructureNSC(NegSkyStrAux &structure, ListVectorListUSetDualSpace &list_Vc_lt_UsDs, Space d){
    Space all=(1<<d)-1;
    long structSize=0;
    //DataType i;
    DataType nbTuples=0;
  
    // auto iterator_list=list_Vc_lt_UsDs.begin() ;
    // if (list_Vc_lt_UsDs.size()>1) iterator_list++;
    auto iterator_list=list_Vc_lt_UsDs.rbegin();//iterator_list++;
    int j=0;
    while (iterator_list!=list_Vc_lt_UsDs.rend()){
        for (int i=0;i<iterator_list->size();++i){ // on boucle sur tous les tuples taille n
            //if (listUSetDualSpace[i].size()!=1 || (listUSetDualSpace[i].begin())->dom<all){  
                DataType idTuple=nbTuples;
                auto it_us=(*iterator_list)[i].rbegin();//it_us++;
                for (;it_us!=(*iterator_list)[i].rend();++it_us){
                    
                    for (auto it=(*it_us).begin();it!=(*it_us).end();++it){ // on boucle sur tous les paris (X|Y) de ce tuple
                        Space spaceXY=it->dom+it->equ;
                        Space spaceY=it->equ;
                        auto it2=structure.find(spaceXY);
                        if (it2==structure.end()){
                            unordered_map<Space,vector<DataType>> mapAux;
                            vector<DataType> vectAux;
                            vectAux.push_back(idTuple);
                            mapAux.insert(pair<Space,vector<DataType>>(spaceY,vectAux));
                            structure.insert(pair<Space,unordered_map<Space,vector<DataType>>>(spaceXY,mapAux));
                        }else{
                            auto it3=(it2->second).find(spaceY);
                            if (it3==(it2->second).end()){
                                vector<DataType> vectAux;
                                vectAux.push_back(idTuple);
                                (it2->second).insert(pair<Space,vector<DataType>>(spaceY,vectAux));
                            }else{
                                (it3->second).push_back(idTuple);
                            }
                        }
                        structSize++;
                    }
               
                }
                nbTuples++;j++;
            //}
        }
        iterator_list++;
    }
    return structSize;
}

// down     ,  transform NegSkyStrAux to NegSkyStr,   map -> vector
long negativeSkycube(NegSkyStr &structure, ListVectorListUSetDualSpace &list_Vc_lt_UsDs, Space d){  
     long structSize;   
    Space spXY, spY;    
     NegSkyStrAux structure0;   
    structSize = creationStructureNSC(structure0, list_Vc_lt_UsDs, d);  
   
    for (auto itXY=structure0.begin();itXY!=structure0.end();itXY++){   
        spXY=itXY->first;   
        vector<pair<Space,vector<DataType>>> vY;    
        for (auto itY=(itXY->second).begin();itY!=(itXY->second).end();itY++){  
            spY=itY->first; 
            vector<Space> vId;  
            for (auto itId=(itY->second).begin();itId!=(itY->second).end();itId++){ 
                vId.push_back(*itId);   
            }   
            vY.push_back(pair<Space, vector<DataType>>(spY, vId));  
        }   
        structure.push_back(pair<Space, vector<pair<Space,vector<DataType>>>>(spXY, vY));   
    }   

    return structSize;  
}

bool pet(const pair<Space, TableTuple> &p1, const pair<Space, TableTuple> &p2 ){
    return __builtin_popcount(p1.first) >__builtin_popcount(p2.first);
}

void choixPivot(TableTuple &topmost, Space d){
    DataType n= topmost.size();
    DataType iMinMax=-1, minMax=n, maxVal;
    for (auto i=0;i<n;i++){
        maxVal=topmost[i][0];
        for (auto j=1;j<d;j++) if (maxVal<topmost[i][j]) maxVal=topmost[i][j];
        if (maxVal<minMax){
            iMinMax=i;
            minMax=maxVal;
        }
    }
    Point pointAux=topmost[0];
    topmost[0]=topmost[iMinMax];
    topmost[iMinMax]=pointAux;
}







bool* subspaceSkyline_NSC(NegSkyStr &structure, int size_data, const Space subspace){
    DataType i;
    bool* tableSkyline=new bool[size_data];
    for (i=0;i<size_data;i++) tableSkyline[i]=true;
    //for (auto itXY=structure.rbegin();itXY!=structure.rend() && subspace<=(itXY->first);++itXY){
    for (auto itXY=structure.rbegin();itXY!=structure.rend();++itXY){
        if (estInclusDans(subspace, itXY->first)){
            for (auto itY=(itXY->second).begin();itY!=(itXY->second).end();++itY) {
                if (!estInclusDans(subspace, itY->first)) {
                    for (auto itId=(itY->second).begin();itId!=(itY->second).end();++itId){
                        tableSkyline[*itId]=false;
                    }
                }
            }
        }
    }

    return tableSkyline;
}

bool* subspaceSkylineSize_NSC(NegSkyStr &structure, int size_data, Space subspace){
    DataType skySize;

    bool* skyline=subspaceSkyline_NSC(structure, size_data, subspace);
    skySize=compteLesMarques(skyline, size_data);
    //delete[] skyline;
    
    return skyline;

}


DataType compteLesMarques(bool* table, DataType length){
/*  function qui compte le nombre de cellules valant 'vrai' dans une table de booleens
    utile pour calculer la taille du skyline*/
    DataType i, total=0;
    for (i=0; i<length; i++) total+=table[i];
    return total;
}



void displayResult(string dataName, DataType n, Space d, DataType k, string step, long structSize, double timeToPerform){
     cerr<<dataName<<" "<<n<<" "<<d<<" "<<k<<" "<<"NSCt"<<" "<<step<<" "<<structSize<<" "<<timeToPerform<<endl;
}


void skylinequery(string dataName, NegSkyStr &structure0,  int indexedDataSize, Space d, DataType k, vector<Space> subspaceN, TableTuple &donnees, vector<vector<Space>> &vectSpaceN, int indexTime){

    Space All=(1<<d)-1;
    int N=subspaceN.size();
    string nombreRequete ="N="+to_string(N);

    int structSize=0;
    double timeToPerform;
    double timeToPerform_NSC;
    TableTuple interSky;
    timeToPerform=debut();
    double temps=0;
    for (int i=0; i<N; i++){

        timeToPerform_NSC=debut();
        bool *skyline_NSC;
        skyline_NSC=subspaceSkylineSize_NSC(structure0, indexedDataSize, subspaceN[i]);
        
        for (int j=0;j<indexedDataSize;j++){
            if (skyline_NSC[j]==true) {
  
                structSize++;
            }
        }

    }
    timeToPerform=duree(timeToPerform);
    displayResult(dataName, indexedDataSize, d, k, nombreRequete, structSize, timeToPerform); 

}




void updateNSCt_step1(TableTuple &buffer, list<TableTuple> &mainTopmost, ListVectorListUSetDualSpace &ltVcLtUsDs, Space d){
    Space All=(1<<d)-1;
    int count=0;
    //On calcule les paires des nouveaux tuples
    VectorListUSetDualSpace buffer_pairs(buffer.size());
    //Pour chaque tuple t dans buffer, i.e., qu'on veut insérer
    #pragma omp parallel for num_threads(NB_THREADS) schedule(dynamic) reduction(+:count)
    for (int k=0; k<buffer.size(); k++){
        bool all_yes=false;
        vector<list<DualSpace>> pairsToCompress(mainTopmost.size());  // structure temporaire pour compresser les pairs en cascade
        //Pour chaque topmost de chaque bucket
        for (auto it_topmost = mainTopmost.begin(); it_topmost!=mainTopmost.end(); it_topmost++){
             USetDualSpace usDs;
            //Pour chaque tuple t' dans le topmost actuel, comparer t à t'
             for (int j=0; j <(*it_topmost).size();j++){
                 DualSpace ds;
                 ds=NEG::domDualSubspace_1((*it_topmost)[j], buffer[k], d);
                 if (ds.dom==All){all_yes=true;}
                 usDs.insert(ds);//on met la paire dans un ensemble spécifique au bucket courant
             }   
            //On affecte l'ensemble de paires à une liste
            list<DualSpace> maListe;
            for(auto it=usDs.begin(); it!=usDs.end(); ++it)maListe.push_back(*it);
            pairsToCompress.push_back(maListe);     
        }
        //Une fois tous les buckets de paires de t obtenues, on les compresse.
        CompresserParInclusion_cascade(pairsToCompress,d,buffer_pairs[k]);
        if(all_yes){count++;}
    }

    ltVcLtUsDs.push_front(buffer_pairs); //bucket des paires des new tuples 
    cerr<< "number of All: "<<count<<endl;
}

void updateNSCt_step2(TableTuple &topmostBuffer, list<TableTuple> &mainDataset, ListVectorListUSetDualSpace &ltVcLtUsDs, Space d){
    Space All=(1<<d)-1;
    int count=0;
    //On met à jour la liste de buckets de paires associée aux anciens tuples
    
    if(ltVcLtUsDs.size()>1){ // if it is not the case, then there is nothing to update

        auto it_bloc_data=mainDataset.begin();
        auto it_bloc_pair=ltVcLtUsDs.begin();
        it_bloc_data++;it_bloc_pair++; // the first one does not need to be updated

        // on boucle sur tous les blocs de tuples existants
        int block_position=0;
        while (it_bloc_data!=mainDataset.end()){

            #pragma omp parallel for num_threads(NB_THREADS) schedule(dynamic) reduction(+:count)
    
            // on boucle sur tous les tuples d'un bloc 
            for (int i=0; i<(*it_bloc_data).size(); i++){
                bool all_yes=false;
                USetDualSpace usDS;
                // on le compare au topmost des new_tuples
                for (int j=0; j <topmostBuffer.size();j++){
                    DualSpace ds;
                    ds=NEG::domDualSubspace_1(topmostBuffer[j], (*it_bloc_data)[i], d);
                    if (ds.dom==All){all_yes=true;}
                    usDS.insert(ds);
    
                }

                // Pour compression cascade
                vector<list<DualSpace>> pairsToCompress;

                list<DualSpace> maListe;
                for(auto it=usDS.begin(); it!=usDS.end(); ++it)maListe.push_back(*it);
                
                pairsToCompress.push_back(maListe);

                for (auto it_list = (*it_bloc_pair)[i].begin() ; it_list!=(*it_bloc_pair)[i].end();it_list++){
                    list<DualSpace> pairs;
                    for(auto it_us = it_list->begin(); it_us!=it_list->end();it_us++){
                        pairs.push_back(*it_us);
                    }  
                    pairsToCompress.push_back(pairs);
                }
                

                // Compression locale
        
                // list<DualSpace> maListe;
                // for(auto it=usDS.begin(); it!=usDS.end(); ++it)maListe.push_back(*it);


                // CompresserParInclusion(maListe);
                // usDS.clear();
                // usDS.insert(maListe.begin(),maListe.end());
    
                // fusionGloutonne(usDS,d);
            
                //on append les nouvelles paires
                
                //(*it_bloc_pair)[i].push_front(usDS);
                (*it_bloc_pair)[i]=CompresserParInclusion_cascade_v2(pairsToCompress,d,block_position);

                if(all_yes){count++;}
            }
            it_bloc_data++;
            it_bloc_pair++;
            block_position++;
        }
        cerr<< "number of All: "<<count<<endl;
    
    }   
       
}





void expiration(ListVectorListUSetDualSpace &listVcUSetDualSpace){

    for (auto it_list = listVcUSetDualSpace.begin(); it_list != listVcUSetDualSpace.end(); it_list++){
        for (auto it_vector = (*it_list).begin(); it_vector!=(*it_list).end(); it_vector++){

            (*it_vector).pop_back();
        }
    }
  
}


void CompresserParInclusion(list<DualSpace> &l){
    l.sort(NEG::pet_pair);
    for(auto it=l.begin(); it!=l.end();it++){
        auto it1=it;
        it1++;
        while(it1!=l.end() ){
            if (estInclusDans((*it1).dom,(*it).dom) && (estInclusDans((*it1).equ, (*it).dom + (*it).equ))){
                it1=l.erase(it1);
            }
            else{
                it1++;
            }
        }
    }
}

void compresserParInclusion2liste(list<DualSpace> &l,list<DualSpace> &s){

l.sort(NEG::pet_pair);
s.sort(NEG::pet_pair);

    auto it=l.begin();
    while(it!=l.end()){
        auto it1=s.begin();
        while(it1!=s.end()){
            if (estInclusDans((*it).dom,(*it1).dom) && (estInclusDans((*it).equ, (*it1).dom + (*it1).equ))){
                it=l.erase(it);
                break;
            }
            it1++;
        }
        if(it1==s.end()){
            it++;
        }

      
    }



}

void CompresserParInclusion_cascade(vector<list<DualSpace>> &toCompress, Space d, ListUSetDualSpace &l){


  //  ListUSetDualSpace l;

    for (int i=0;i<toCompress.size();i++){ // the more recent buck  is in the front of the vector

        USetDualSpace usDs;

        CompresserParInclusion(toCompress[i]);

        list<DualSpace> lds;

        for (int j=0; j<i;j++){  // we gather more recent buck, i.e. j smaller than i  
            for (auto it=toCompress[j].begin(); it!=toCompress[j].end();it++){
                lds.push_front(*it);
            }
            
        }

        compresserParInclusion2liste(toCompress[i],lds);
        
        usDs.insert(toCompress[i].begin(),toCompress[i].end());

        fusionGloutonne(usDs, d);// meilleure position pour fusionGloutonne, mettre ici ou enlever completement

        l.push_back(usDs);
    }

 //   return l;
}

ListUSetDualSpace CompresserParInclusion_cascade_v2(vector<list<DualSpace>> &toCompress, Space d, int buck_position){


    ListUSetDualSpace l;

    int buck_processed=0;

    for (int i=0;i<toCompress.size();i++){ // the more recent buck  is in the front of the vector

        if (buck_processed<=buck_position+2){
        
            if (i==0){
                USetDualSpace usDs;

                CompresserParInclusion(toCompress[i]);

                list<DualSpace> lds;

                for (int j=0; j<=buck_position+1;j++){  // we gather more recent buck, i.e. j smaller than i  
                    if (j!=i){
                        for (auto it=toCompress[j].begin(); it!=toCompress[j].end();it++){
                            lds.push_front(*it);
                        }
                    }
                }

                compresserParInclusion2liste(toCompress[i],lds);
        
                usDs.insert(toCompress[i].begin(),toCompress[i].end());

                fusionGloutonne(usDs, d);// meilleure position pour fusionGloutonne, mettre ici ou enlever completement

                l.push_back(usDs);
            
            }
            else{

                USetDualSpace usDs;

                list<DualSpace> lds;

                for (int j=0; j<=buck_position+1;j++){  // we gather more recent buck, i.e. j smaller than i  
                    if (j!=i){
                        for (auto it=toCompress[j].begin(); it!=toCompress[j].end();it++){
                            lds.push_front(*it);
                        }
                    }
                }

                compresserParInclusion2liste(toCompress[i],lds);
        
                usDs.insert(toCompress[i].begin(),toCompress[i].end());

                l.push_back(usDs);
            
            }
            
        }
        else{

            USetDualSpace usDs;

            list<DualSpace> lds;

            for (int j=0; j<1;j++){  // we gather more recent buck, i.e. j smaller than i  
                
                for (auto it=toCompress[j].begin(); it!=toCompress[j].end();it++){
                    lds.push_front(*it);
                }
               
            }

            compresserParInclusion2liste(toCompress[i],lds);
        
            usDs.insert(toCompress[i].begin(),toCompress[i].end());

            l.push_back(usDs);
        }

        buck_processed++;
    }

    return l;
}

void InitStructure (list<TableTuple> &mainDataset, list<TableTuple> &mainTopmost, ListVectorListUSetDualSpace &ltVcLtUsDs, Space d ){

    int block_position=0;

    for (auto it_listDataset=mainDataset.rbegin();it_listDataset!=mainDataset.rend();it_listDataset++){

        VectorListUSetDualSpace bloc_pairs(it_listDataset->size());

        #pragma omp parallel for num_threads(NB_THREADS) schedule(dynamic)
        
        for (int i = 0; i<it_listDataset->size();i++){

            vector<list<DualSpace>> pairsToCompress(mainTopmost.size());  // structure temporaire pour compresser les pairs en cascade

            for (auto it_topmost = mainTopmost.begin(); it_topmost!=mainTopmost.end(); it_topmost++){

                USetDualSpace usDs;

                for (int j=0; j <(*it_topmost).size();j++){
                    DualSpace ds;
                    ds=NEG::domDualSubspace_1((*it_topmost)[j], (*it_listDataset)[i], d);
                    usDs.insert(ds);
                }   

                list<DualSpace> maListe;
                for(auto it=usDs.begin(); it!=usDs.end(); ++it)maListe.push_back(*it);
 
                pairsToCompress.push_back(maListe);     
             
            }
           
            CompresserParInclusion_cascade(pairsToCompress,d,bloc_pairs[i]);

        }

        ltVcLtUsDs.push_front(bloc_pairs); //bucket des paires des new tuples 

        block_position++;

    }


}

}

