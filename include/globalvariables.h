#pragma once

#include <iostream>
#include <string>

#include "csv.h"
#include <vector>

namespace EquationData {

double g_perm = 1e-6 / 1000 / 9.8;  // permeability
const double g_c_T = 1.2e6;         // heat capacity of the mixture
const double g_lam = 1.2;           // heat conductivity
const double g_c_w = 1e6;           // heat capacity of water
const double g_B_w = 1e5;           // bulk modulus of pores

// Pressure seetings
// const int g_num_P_bnd_id = 8;  // numbers of  pressure boudnary condition id
// const int g_P_bnd_id[g_num_P_bnd_id] = {
//     1, 2, 4, 5, 7, 8, 1613, 1614};   // pressure boundary condition id

const int g_num_P_bnd_id = 8;  // numbers of  pressure boudnary condition id
const int g_P_bnd_id[g_num_P_bnd_id] = {
    1, 2, 4, 5, 7, 8, 1611, 1612};  // pressure boundary condition id

const double g_Pb_top = 0;           // pressure at the top of model
const double g_P_grad = 1000 * 9.8;  // pressure gradient in vertial direction

// Velocity settings
// const int g_num_QP_bnd_id = 4;  // numbers of velocity boudnary condition id
// const int g_QP_bnd_id[g_num_QP_bnd_id] = {1, 4, 1613,
//                                           1614};  // velocity  boundary

const int g_num_QP_bnd_id = 4;  // numbers of velocity boudnary condition id
const int g_QP_bnd_id[g_num_QP_bnd_id] = {1, 4, 1611,
                                          1612};  // velocity  boundary

const double g_Qb_lateral = 0;  // wellbore temperature

// Temperature seetings
// const int g_num_T_bnd_id = 813;  // numbers of  temperature boudnary
// condition id const int g_T_bnd_id[g_num_T_bnd_id] = {
//     15,   16,   19,   20,   23,   24,   27,   28,   31,   32,   35,   36,
//     39,   40,   43,   44,   47,   48,   51,   52,   54,   56,   58,   60,
//     62,   64,   66,   68,   70,   72,   74,   76,   78,   80,   82,   84,
//     86,   88,   90,   92,   95,   96,   99,   100,  103,  104,  107,  108,
//     111,  112,  115,  116,  119,  120,  123,  124,  127,  128,  131,  132,
//     134,  136,  138,  140,  142,  144,  146,  148,  150,  152,  154,  156,
//     158,  160,  162,  164,  166,  168,  170,  172,  175,  176,  179,  180,
//     183,  184,  187,  188,  191,  192,  195,  196,  199,  200,  203,  204,
//     207,  208,  211,  212,  214,  216,  218,  220,  222,  224,  226,  228,
//     230,  232,  234,  236,  238,  240,  242,  244,  246,  248,  250,  252,
//     255,  256,  259,  260,  263,  264,  267,  268,  271,  272,  275,  276,
//     279,  280,  283,  284,  287,  288,  291,  292,  294,  296,  298,  300,
//     302,  304,  306,  308,  310,  312,  314,  316,  318,  320,  322,  324,
//     326,  328,  330,  332,  335,  336,  339,  340,  343,  344,  347,  348,
//     351,  352,  355,  356,  359,  360,  363,  364,  367,  368,  371,  372,
//     374,  376,  378,  380,  382,  384,  386,  388,  390,  392,  394,  396,
//     398,  400,  402,  404,  406,  408,  410,  412,  415,  416,  419,  420,
//     423,  424,  427,  428,  431,  432,  435,  436,  439,  440,  443,  444,
//     447,  448,  451,  452,  454,  456,  458,  460,  462,  464,  466,  468,
//     470,  472,  474,  476,  478,  480,  482,  484,  486,  488,  490,  492,
//     495,  496,  499,  500,  503,  504,  507,  508,  511,  512,  515,  516,
//     519,  520,  523,  524,  527,  528,  531,  532,  534,  536,  538,  540,
//     542,  544,  546,  548,  550,  552,  554,  556,  558,  560,  562,  564,
//     566,  568,  570,  572,  575,  576,  579,  580,  583,  584,  587,  588,
//     591,  592,  595,  596,  599,  600,  603,  604,  607,  608,  611,  612,
//     614,  616,  618,  620,  622,  624,  626,  628,  630,  632,  634,  636,
//     638,  640,  642,  644,  646,  648,  650,  652,  655,  656,  659,  660,
//     663,  664,  667,  668,  671,  672,  675,  676,  679,  680,  683,  684,
//     687,  688,  691,  692,  694,  696,  698,  700,  702,  704,  706,  708,
//     710,  712,  714,  716,  718,  720,  722,  724,  726,  728,  730,  732,
//     735,  736,  739,  740,  743,  744,  747,  748,  751,  752,  755,  756,
//     759,  760,  763,  764,  767,  768,  771,  772,  774,  776,  778,  780,
//     782,  784,  786,  788,  790,  792,  794,  796,  798,  800,  802,  804,
//     806,  808,  810,  812,  815,  816,  819,  820,  823,  824,  827,  828,
//     831,  832,  835,  836,  839,  840,  843,  844,  847,  848,  851,  852,
//     854,  856,  858,  860,  862,  864,  866,  868,  870,  872,  874,  876,
//     878,  880,  882,  884,  886,  888,  890,  892,  895,  896,  899,  900,
//     903,  904,  907,  908,  911,  912,  915,  916,  919,  920,  923,  924,
//     927,  928,  931,  932,  934,  936,  938,  940,  942,  944,  946,  948,
//     950,  952,  954,  956,  958,  960,  962,  964,  966,  968,  970,  972,
//     975,  976,  979,  980,  983,  984,  987,  988,  991,  992,  995,  996,
//     999,  1000, 1003, 1004, 1007, 1008, 1011, 1012, 1014, 1016, 1018, 1020,
//     1022, 1024, 1026, 1028, 1030, 1032, 1034, 1036, 1038, 1040, 1042, 1044,
//     1046, 1048, 1050, 1052, 1055, 1056, 1059, 1060, 1063, 1064, 1067, 1068,
//     1071, 1072, 1075, 1076, 1079, 1080, 1083, 1084, 1087, 1088, 1091, 1092,
//     1094, 1096, 1098, 1100, 1102, 1104, 1106, 1108, 1110, 1112, 1114, 1116,
//     1118, 1120, 1122, 1124, 1126, 1128, 1130, 1132, 1135, 1136, 1139, 1140,
//     1143, 1144, 1147, 1148, 1151, 1152, 1155, 1156, 1159, 1160, 1163, 1164,
//     1167, 1168, 1171, 1172, 1174, 1176, 1178, 1180, 1182, 1184, 1186, 1188,
//     1190, 1192, 1194, 1196, 1198, 1200, 1202, 1204, 1206, 1208, 1210, 1212,
//     1215, 1216, 1219, 1220, 1223, 1224, 1227, 1228, 1231, 1232, 1235, 1236,
//     1239, 1240, 1243, 1244, 1247, 1248, 1251, 1252, 1254, 1256, 1258, 1260,
//     1262, 1264, 1266, 1268, 1270, 1272, 1274, 1276, 1278, 1280, 1282, 1284,
//     1286, 1288, 1290, 1292, 1295, 1296, 1299, 1300, 1303, 1304, 1307, 1308,
//     1311, 1312, 1315, 1316, 1319, 1320, 1323, 1324, 1327, 1328, 1331, 1332,
//     1334, 1336, 1338, 1340, 1342, 1344, 1346, 1348, 1350, 1352, 1354, 1356,
//     1358, 1360, 1362, 1364, 1366, 1368, 1370, 1372, 1375, 1376, 1379, 1380,
//     1383, 1384, 1387, 1388, 1391, 1392, 1395, 1396, 1399, 1400, 1403, 1404,
//     1407, 1408, 1411, 1412, 1414, 1416, 1418, 1420, 1422, 1424, 1426, 1428,
//     1430, 1432, 1434, 1436, 1438, 1440, 1442, 1444, 1446, 1448, 1450, 1452,
//     1455, 1456, 1459, 1460, 1463, 1464, 1467, 1468, 1471, 1472, 1475, 1476,
//     1479, 1480, 1483, 1484, 1487, 1488, 1491, 1492, 1494, 1496, 1498, 1500,
//     1502, 1504, 1506, 1508, 1510, 1512, 1514, 1516, 1518, 1520, 1522, 1524,
//     1526, 1528, 1530, 1532, 1535, 1536, 1539, 1540, 1543, 1544, 1547, 1548,
//     1551, 1552, 1555, 1556, 1559, 1560, 1563, 1564, 1567, 1568, 1571, 1572,
//     1574, 1576, 1578, 1580, 1582, 1584, 1586, 1588, 1590, 1592, 1594, 1596,
//     1598, 1600, 1602, 1604, 1606, 1608, 1610, 1612, 1,    2,    4,    5,
//     7,    8,    1613, 1614, 6,    10,   3,    9,    11};  //  temperature

// for course case
// const int g_num_T_bnd_id = 9;
// const int g_T_bnd_id[g_num_T_bnd_id] = {15, 1,    2,    4,    5,
//      7,    8,    1613, 1614};

// for fine case
const int g_num_T_bnd_id = 13;
const int g_T_bnd_id[g_num_T_bnd_id] = {13, 10, 9, 3, 6,    1,   2,
                                        4,  5,  7, 8, 1611, 1612};

const double g_Tb_well = 273.15 + 25;  // wellbore temperature
const double g_Tb_top = 273.15 + 15;   // termperature at the top of model
const double g_Tb_bottom = 273.15 + 18;
const double g_Tb_seabed_top = 273.15 + 5;  // termperature at the top of model
const double g_Tb_seabed_bottom = g_Tb_bottom;
const double g_T_grad = (g_Tb_bottom - g_Tb_top) /
                        60.96;  // temperature gradient in verital direction
const double g_T_seabed_grad = (g_Tb_seabed_bottom - g_Tb_seabed_top) / 60.96;

// Heat flow rate settins
// const int g_num_QT_bnd_id = 6;
// const int g_QT_bnd_id[g_num_QT_bnd_id] = {6, 10, 12,
//                                           3, 9,  11};

const int g_num_QT_bnd_id = 1;  // numbers of velocity boudnary condition id
const int g_QT_bnd_id[g_num_QT_bnd_id] = {10};  // velocity  boundary

const double g_QT_well = 0;  // wellbore temperature
const double g_QT_top = -g_lam * g_T_grad;
const double g_QT_bottom = g_lam * g_T_grad;

// solver settings

const bool is_linspace = false;
const double g_period = 86400 * 180;  // simulation time
const int g_n_time_step = 15;         // simulation time
std::vector<double> g_time_sequence = {0,  0.1, 0.2, 0.5, 1,   2,   5,  10,
                                       15, 30,  60,  90,  120, 150, 180};
const char time_unit = 'd';
const unsigned int n_g_P_max_iteration = 1000;
const unsigned int n_g_T_max_iteration = 4000;
const double g_P_tol_residual = 1e-8;
const double g_T_tol_residual = 1e-10;

// dimention of the input data file (parameters_for_interpolation.txt in
// inputfiles is used in the example)
const int dimension = 3;
// dimension in x, y and z directions
std::string file_name_interpolation =
    "inputfiles/parameters_for_interpolation.txt";
// number of sample point in x directin, y direction and z direction
const int dimension_x = 2;
const int dimension_y = 2;
const int dimension_z = 7;

}  // namespace EquationData
