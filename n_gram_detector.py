import numpy as np
from os import listdir
from os.path import isfile, isdir, join
import csv
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
from sklearn.svm import SVC
import xgboost as xgb
import time
from sklearn.preprocessing import OneHotEncoder
from sklearn.metrics import confusion_matrix
from sklearn.preprocessing import MinMaxScaler
import pandas as pd
from collections import Counter
from sklearn.metrics import precision_recall_fscore_support
import math
start_time = time.time()
csv_file_path = '/home/weiren/ML_experiment_all_mal/Filtered_armel_Malware_newLabel.csv'
# 讀取 CSV 文件
data = pd.read_csv(csv_file_path)
filename_to_number_mapping = {}
filename_to_label_mapping = pd.Series(data['label'].values, index=data['filename']).to_dict()
labels, unique_labels = pd.factorize(data['label'])

# 將這些整數標籤添加到原始的 DataFrame
data['label_mapped'] = labels
# 創建從 filename 到整數標籤的映射字典
filename_to_label_mapping = pd.Series(data['label_mapped'].values, index=data['filename']).to_dict()
max_n_gram_appear = [1] * 4096
path = "/home/weiren/ML_experiment_all_mal/testing_gram/"
files = listdir(path)
mal_score = []
for f in files:
    fullpath = join(path, f)
    with open(fullpath,'r') as contents:
        for line in contents:
            line_cont = line.strip().split(' ')
#             print(line_cont)
            if line_cont[0] not in filename_to_number_mapping:
                filename_to_number_mapping[line_cont[0]] = len(filename_to_number_mapping)
            max_n_gram_appear[int(line_cont[1])-1] = max(max_n_gram_appear[int(line_cont[1])-1],int(line_cont[2]))

each_mal_score = []

for f in files:
    fullpath = join(path, f)
    mal_score = [] #這裡是把每筆file跟用來所有用來建樹的file的分數。
    with open(fullpath) as mal_file:
        temp = mal_file.readlines()
        for row in temp:
            row = row.split(" ")
#             print(row)
            score = 0
            each_score=[] #這邊是紀錄每個file跟target file的分數
            for i in range(1,len(row)-2,2):
                # score += float(row[i])/float(n_gram_max[int(row[i])])*float(row[i+1]) #權重*出現次數
                score += float(row[i])*(float(row[i+1]))
                # score += float(row[i])*(float(row[i+1])/float(max_n_gram_appear[int(row[i])-1]))
                # score += (float(row[i])**(2.5))*math.log(1+int(row[i+1])) 
                # score += (1.0 / (float(row[i]) ** 0.5))*(float(row[i+1])/float(max_n_gram_appear[int(row[i])-1]))

            each_score.append(row[0])
            each_score.append(score)
            mal_score.append(each_score)
        mal_score = sorted(mal_score, key=lambda x: x[1], reverse=True)
        each_mal_score.append([f,mal_score])
# print(each_mal_score)
each_mal_score[0][1][:3]
for choose in range(1,2):
    print('choose= ',choose)
    correct = 0
    wrong = 0
    y_true = []
    y_pred = []
    for i in range(len(each_mal_score)):
        # choose = int(min(max(3,len(mal_score[i])*0.005),20))
        # choose = 4
        family_cal = {}  
        for file_name, value in each_mal_score[i][1][:choose]:
    #         print(file_name,value)
    #         print(filename_to_label_mapping[file_name[:-4]])
            if filename_to_label_mapping[file_name[:-4]] not in family_cal:
                # family_cal[filename_to_label_mapping[file_name[:-4]]] = 1
                family_cal[filename_to_label_mapping[file_name[:-4]]] = value
            else:
                # family_cal[filename_to_label_mapping[file_name[:-4]]] += 1
                family_cal[filename_to_label_mapping[file_name[:-4]]] += value
        key_with_max_value = max(family_cal, key=family_cal.get)
        if unique_labels[filename_to_label_mapping[each_mal_score[i][0][:-15]]] == unique_labels[key_with_max_value]:
            correct+=1
        else:
            wrong+=1
    #     print('原始的malware = ',unique_labels[filename_to_label_mapping[each_mal_score[i][0][:-15]]], ' 預測的malware = ',unique_labels[key_with_max_value])
    #     output_string = f"原始的malware = {unique_labels[filename_to_label_mapping[each_mal_score[i][0][:-15]]]}, 預測的malware = {unique_labels[key_with_max_value]}\n"
    #     with open('output.txt', 'a+', encoding='utf-8') as file:
    #         file.write(output_string)
        y_true.append(filename_to_label_mapping[each_mal_score[i][0][:-15]])
        y_pred.append(key_with_max_value)
    print('correct= ',correct)
    print('wrong= ',wrong)
    print('Accuracy = ',float(correct)/float(correct+wrong))
    accuracy = float(correct)/float(correct+wrong)
    # with open('/home/weiren/cout_log.txt', 'a+', encoding='utf-8') as file:
    #     file.write(f"Detector= {accuracy}\n")

        # 使用 sklearn 计算混淆矩阵
    conf_matrix = confusion_matrix(y_true, y_pred)

    # Get unique labels (class names)
    unique_labels = pd.factorize(data['label'])[1]

    # Print the confusion matrix with labels for better readability
    conf_matrix_df = pd.DataFrame(conf_matrix, index=unique_labels, columns=unique_labels)
    precision, recall, f1_score, _ = precision_recall_fscore_support(y_true, y_pred, average='macro')
    print('Precision= ',precision)
    print('Recall= ',recall)
    print('F1_score= ',f1_score)

    pd.set_option('display.max_rows', None)  # None表示不限制行数
    pd.set_option('display.max_columns', None)  # None表示不限制列数
    # 可能还需要调整行和列的宽度，根据你的具体情况
    pd.set_option('display.width', 1000)  # 增加总体宽度以适应更多列
    print(conf_matrix_df)
    end_time = time.time()
    elapsed_time = end_time - start_time  # 計算經過時間
    print('Predict Time = ', elapsed_time,'\n')
    # print(f"The code took {elapsed_time} seconds to run.")