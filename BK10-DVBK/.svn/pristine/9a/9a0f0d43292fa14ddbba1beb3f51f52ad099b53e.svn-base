#include <ql_oe.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gps.h"
#include "network.h"
#include "log.h"
#include "database.h"
#include "driver.h"
#include "devconfig.h"
#include "fw.h"
#include "store.h"
#include "encrypt.h"
#include "filter.h"
#include "camera.h"
#include "setting.h"

#define TOTOL_AREA 16
//Data Area 1: Cao toc Ha Noi - Hai Phong
const float Area_1[122]= {
	150, 60,
	106.750366,20.842211,
	106.748631,20.822473,
	106.7508622,20.7904105,
	106.722909,20.775079,
	106.693922,20.767092,
	106.649588,20.767005,
	106.621639,20.771606,
	106.592022,20.77184,
	106.581384,20.774728,
	106.547156,20.795449,
	106.5221872,20.8061126,
	106.424191,20.839341,
	106.398455,20.843192,
	106.350384,20.858168,
	106.336547,20.85989,
	106.314622,20.857495,
	106.278369,20.864087,
	106.265601,20.864787,
	106.217421,20.861218,
	106.193962,20.855829,
	106.160622,20.856525,
	106.0941296,20.8490101,
	106.0453219,20.8469302,
	106.03268,20.853863,
	106.0208867,20.8648979,
	106.003855,20.881305,
	105.973273,20.931471,
	105.944914,20.984025,
	105.921609,21.010801,
	105.919631,21.009839,
	105.943146,20.982715,
	105.968699,20.934882,
	105.987342,20.901469,
	106.00153,20.8794134,
	106.03122,20.852217,
	106.041546,20.846023,
	106.047897,20.844484,
	106.087743,20.84613,
	106.136741,20.85064,
	106.160838,20.854335,
	106.1964413,20.8534706,
	106.217859,20.859062,
	106.265759,20.862593,
	106.314418,20.855305,
	106.336593,20.85769,
	106.349856,20.856032,
	106.3955188,20.8406068,
	106.423789,20.837179,
	106.521507,20.803489,
	106.546044,20.793551,
	106.580236,20.772852,
	106.587544,20.770013,
	106.649372,20.764815,
	106.691237,20.764455,
	106.7159062,20.7701521,
	106.7555682,20.7874637,
	106.750829,20.822547,
	106.752554,20.841989,
	106.752538,20.842505,
	106.750366,20.842211};

//Data Area 2: Cao toc Bac Giang - Lang Son
const float Area_2[126]= {
	150, 62,
	106.62337,21.684265,
	106.604495,21.670189,
	106.596054,21.658303,
	106.583485,21.650086,
	106.556615,21.622093,
	106.544785,21.613408,
	106.527933,21.594245,
	106.518562,21.577872,
	106.4960433,21.5599213,
	106.483504,21.553021,
	106.458663,21.547567,
	106.448824,21.54128,
	106.438038,21.54131,
	106.425434,21.533852,
	106.411622,21.530658,
	106.396332,21.517709,
	106.390607,21.505366,
	106.37646,21.495492,
	106.368267,21.481215,
	106.351789,21.466213,
	106.321926,21.433396,
	106.306686,21.424501,
	106.296731,21.404033,
	106.296505,21.387287,
	106.286791,21.352627,
	106.2720245,21.3404937,
	106.266666,21.322821,
	106.253892,21.314933,
	106.233092,21.292063,
	106.234865,21.29076,
	106.2483094,21.304008,
	106.255318,21.313257,
	106.267253,21.319592,
	106.2729876,21.339064,
	106.283859,21.345654,
	106.288796,21.35172,
	106.298645,21.386773,
	106.298923,21.403844,
	106.308447,21.423182,
	106.323194,21.431598,
	106.349604,21.456577,
	106.353863,21.4656622,
	106.369743,21.479585,
	106.37823,21.494185,
	106.3925179,21.503631,
	106.397428,21.514717,
	106.412745,21.528766,
	106.425909,21.531704,
	106.438892,21.539283,
	106.448836,21.53908,
	106.4633922,21.546129,
	106.4900301,21.5522918,
	106.520285,21.576505,
	106.529567,21.592772,
	106.5393764,21.6024373,
	106.546612,21.612182,
	106.558021,21.620401,
	106.584872,21.648378,
	106.59737,21.65654,
	106.606401,21.669091,
	106.624627,21.682459,
	106.62337,21.684265
};

	//Data Area 3: Cao toc Da Nang - Quang Ngai
const float Area_3[126]= {
	150, 62,
	108.167293,15.960084,
	108.170985,15.951597,
	108.174753,15.920815,
	108.174504,15.893786,
	108.175872,15.885676,
	108.19216,15.847831,
	108.192916,15.844752,
	108.192793,15.827468,
	108.194282,15.82163,
	108.21278,15.808228,
	108.2579031,15.788522,
	108.294089,15.753337,
	108.37067,15.662621,
	108.394402,15.619047,
	108.432978,15.550034,
	108.443983,15.53943,
	108.452901,15.534525,
	108.477183,15.527633,
	108.504759,15.514866,
	108.5271161,15.4979083,
	108.57129,15.45355,
	108.620327,15.417367,
	108.641589,15.399206,
	108.6546211,15.3912365,
	108.693567,15.387299,
	108.713226,15.379131,
	108.721763,15.368675,
	108.724731,15.359308,
	108.7244749,15.3254734,
	108.732553,15.309477,
	108.734407,15.310663,
	108.726608,15.330973,
	108.726909,15.359612,
	108.723737,15.369645,
	108.714694,15.380769,
	108.704448,15.386885,
	108.693853,15.389481,
	108.659159,15.3929,
	108.642951,15.400934,
	108.621613,15.419153,
	108.5731364,15.4554426,
	108.5200831,15.5096215,
	108.4806691,15.529013,
	108.453699,15.536575,
	108.445157,15.54129,
	108.434682,15.551426,
	108.396338,15.620093,
	108.3717391,15.6668447,
	108.295711,15.754823,
	108.263786,15.786622,
	108.246697,15.797226,
	108.21388,15.810132,
	108.196058,15.82293,
	108.194987,15.827632,
	108.195104,15.844988,
	108.19426,15.848489,
	108.177968,15.886344,
	108.176696,15.893974,
	108.176947,15.920965,
	108.173135,15.952063,
	108.169267,15.961056,
	108.167293,15.960084
};

	//Data Area 4: Cao toc Ha Noi - Bac Giang
const float Area_4[102]= {
	150, 50,
	106.2303,21.288641,
	106.223405,21.281871,
	106.192179,21.260634,
	106.1808625,21.252436,
	106.1721894,21.248884,
	106.167283,21.24893,
	106.140516,21.252336,
	106.130357,21.250268,
	106.121648,21.246167,
	106.114225,21.240079,
	106.108622,21.232534,
	106.105819,21.226136,
	106.0924006,21.1817393,
	106.0882013,21.1722553,
	106.0825922,21.1677395,
	106.0713,21.164936,
	106.061585,21.159775,
	105.995605,21.117168,
	105.973024,21.103783,
	105.9597548,21.0906118,
	105.9558946,21.0843189,
	105.9550893,21.0669532,
	105.9525575,21.0613661,
	105.930337,21.03047,
	105.9318909,21.0297702,
	105.952816,21.059155,
	105.956593,21.066227,
	105.9570875,21.0824903,
	105.9609498,21.0899021,
	105.9689769,21.0983064,
	105.9769724,21.104244,
	105.996755,21.115292,
	106.062775,21.157925,
	106.07204,21.162864,
	106.084548,21.16694,
	106.090515,21.172384,
	106.0943,21.181302,
	106.107921,21.225484,
	106.110578,21.231526,
	106.115855,21.238601,
	106.122892,21.244353,
	106.1305425,21.2483187,
	106.1397786,21.250442,
	106.1457397,21.25066,
	106.167237,21.24673,
	106.17834,21.249118,
	106.193401,21.258806,
	106.224775,21.280149,
	106.23196,21.287199,
	106.2303,21.288641
};

	//Data Area 5: Cao toc Ha Noi - Ha Long
const float Area_5[126]= {
	150, 62,
	107.015226,20.973958,
	106.996003,20.972037,
	106.982789,20.956784,
	106.9504118,20.9588921,
	106.9153808,20.9846934,
	106.9074819,20.9845061,
	106.8766725,21.0077929,
	106.8589403,21.007365,
	106.8529155,21.0046592,
	106.834553,21.012138,
	106.7907377,21.0333282,
	106.777739,21.0302769,
	106.7604861,21.0328586,
	106.7266019,21.0401029,
	106.672818,21.052336,
	106.641556,21.055358,
	106.617206,21.0521984,
	106.541515,21.07593,
	106.531062,21.074484,
	106.526835,21.084564,
	106.517229,21.08987,
	106.4985105,21.0873208,
	106.458785,21.125124,
	106.437077,21.13321,
	106.4222082,21.1348272,
	106.372912,21.101055,
	106.320504,21.113368,
	106.1988132,21.1286455,
	106.1636651,21.1518872,
	106.1163886,21.1595388,
	106.1012896,21.1704917,
	106.092518,21.1725658,
	106.0922337,21.1714543,
	106.1019583,21.168988,
	106.1138719,21.1591556,
	106.1617343,21.1503753,
	106.176761,21.137604,
	106.203337,21.124616,
	106.2918315,21.1139949,
	106.377807,21.0982048,
	106.4220602,21.1321902,
	106.453149,21.125355,
	106.4979449,21.0848022,
	106.5183591,21.0880504,
	106.5296409,21.0728165,
	106.541465,21.07373,
	106.6153966,21.050541,
	106.6446227,21.0533712,
	106.7343747,21.03707,
	106.735505,21.0356454,
	106.777845,21.028862,
	106.7920468,21.0311922,
	106.8368406,21.007091,
	106.85416,21.00294,
	106.8767776,21.0058651,
	106.9070945,20.982993,
	106.9153046,20.9829268,
	106.9510576,20.9545661,
	106.9864236,20.9543135,
	106.9960968,20.9676382,
	107.016154,20.971962,
	107.015226,20.973958
};

	//Data Area 6: Cao toc Ha Noi - Hoa Binh
const float Area_6[64]= {
	150, 31,
	105.7843792,21.0025679,
	105.7666036,21.0060446,
	105.7577585,21.0081428,
	105.748398,21.0100003,
	105.7188561,21.011698,
	105.7101181,21.0105215,
	105.6916601,21.0062994,
	105.6623851,21.0037796,
	105.646355,21.004627,
	105.599251,21.001344,
	105.5782777,20.9904208,
	105.5711697,20.9895669,
	105.5534696,20.9909281,
	105.5423927,20.9911142,
	105.5312085,20.990939,
	105.5312085,20.9896606,
	105.5424031,20.9896565,
	105.5532758,20.9896115,
	105.5712717,20.9884728,
	105.57795,20.988731,
	105.600149,20.999336,
	105.64199,21.00266,
	105.664468,21.002172,
	105.6915954,21.0051254,
	105.7110118,21.0095497,
	105.7191206,21.0106879,
	105.747082,21.0088999,
	105.7640219,21.0055373,
	105.7793304,21.0023336,
	105.7843106,21.0013615,
	105.7843792,21.0025679
};

	//Data Area 7: Cao toc Ha Noi - Lao Cai
const float Area_7[126]= {
	150, 62,
	105.7317,21.257646,
	105.70979,21.256682,
	105.662914,21.289069,
	105.6478628,21.3225696,
	105.6103997,21.3584722,
	105.5489768,21.366714,
	105.527412,21.355439,
	105.436359,21.359457,
	105.40684,21.36797,
	105.374793,21.369468,
	105.3380779,21.3957081,
	105.303251,21.393545,
	105.2745199,21.4082605,
	105.212024,21.437817,
	105.188611,21.448667,
	105.143716,21.451852,
	105.1024393,21.4459525,
	105.0305843,21.5116039,
	104.950383,21.544695,
	104.929994,21.575939,
	104.9053443,21.6047672,
	104.897955,21.63796,
	104.8812965,21.6770207,
	104.82143,21.742091,
	104.8148068,21.7498189,
	104.4609622,22.0835067,
	104.2999613,22.2240524,
	104.249098,22.268021,
	104.1627178,22.3341945,
	104.0389376,22.3888415,
	104.035254,22.384013,
	104.1060416,22.3356818,
	104.1612372,22.3285697,
	104.33851,22.160969,
	104.5122463,22.0155386,
	104.5941563,21.9265158,
	104.6501613,21.8867767,
	104.792103,21.7446094,
	104.8458291,21.6878634,
	104.881163,21.66783,
	104.901256,21.603072,
	104.948677,21.543305,
	105.0349289,21.4967721,
	105.0969746,21.4340058,
	105.1745361,21.4469723,
	105.240112,21.412796,
	105.302269,21.391575,
	105.3376586,21.3905531,
	105.373827,21.3664529,
	105.4068,21.36577,
	105.435681,21.357363,
	105.527328,21.353241,
	105.6037883,21.3555288,
	105.618561,21.348677,
	105.6441447,21.320668,
	105.660986,21.288011,
	105.698951,21.257931,
	105.70939,21.254518,
	105.73152,21.255454,
	105.7635055,21.238248,
	105.7648128,21.241428,
	105.7317,21.257646
};

	//Data Area 8: Cao toc Ha Noi - Ninh Binh
const float Area_8[120]= {
	150, 59,
	106.012979,20.292108,
	106.001936,20.3166218,
	105.98488,20.344515,
	105.977262,20.400808,
	105.969187,20.442852,
	105.95836,20.48763,
	105.952362,20.497342,
	105.950968,20.504509,
	105.94909,20.538174,
	105.952386,20.57364,
	105.948308,20.612244,
	105.93768,20.660894,
	105.93773,20.670995,
	105.936474,20.678026,
	105.923785,20.69633,
	105.91982,20.706739,
	105.921473,20.746311,
	105.91753,20.76024,
	105.904366,20.780758,
	105.893832,20.805964,
	105.881317,20.870006,
	105.874589,20.880062,
	105.861217,20.913786,
	105.859915,20.944055,
	105.851783,20.956362,
	105.85037,20.96096,
	105.84817,20.961,
	105.849279,20.956357,
	105.858252,20.941651,
	105.859023,20.913614,
	105.860418,20.906534,
	105.872711,20.878918,
	105.879343,20.869034,
	105.880981,20.856047,
	105.88714,20.828202,
	105.8912701,20.8049758,
	105.901017,20.782077,
	105.916748,20.756569,
	105.919287,20.746069,
	105.917591,20.730831,
	105.91764,20.705941,
	105.918348,20.701984,
	105.921995,20.69505,
	105.933557,20.679221,
	105.93548,20.660906,
	105.93751,20.646403,
	105.946172,20.611716,
	105.950194,20.57346,
	105.947052,20.544172,
	105.948772,20.504391,
	105.949614,20.498321,
	105.9564,20.48663,
	105.968773,20.435095,
	105.975098,20.400412,
	105.981921,20.347438,
	105.984221,20.340182,
	106.00536,20.304303,
	106.010961,20.291232,
	106.012979,20.292108
};

	//Data Area 9: Cao toc Ha Noi - Thai Nguyen
const float Area_9[120]= {
	150, 59,
	105.820201,21.568513,
	105.828037,21.543208,
	105.859531,21.511533,
	105.88047,21.500074,
	105.883517,21.474443,
	105.883219,21.448828,
	105.888231,21.426972,
	105.886501,21.409627,
	105.885949,21.38349,
	105.88958,21.370127,
	105.888832,21.36577,
	105.8775233,21.3440744,
	105.87614,21.324137,
	105.8797086,21.3032686,
	105.886194,21.280001,
	105.8910406,21.2681518,
	105.916785,21.250106,
	105.922028,21.244086,
	105.930028,21.219537,
	105.929744,21.193037,
	105.927652,21.187945,
	105.913473,21.171062,
	105.911319,21.166194,
	105.909324,21.146726,
	105.90311,21.131881,
	105.90554,21.126165,
	105.932545,21.10045,
	105.936676,21.090638,
	105.940012,21.086778,
	105.950469,21.084125,
	105.950831,21.086295,
	105.941148,21.088662,
	105.938644,21.091622,
	105.934375,21.10167,
	105.90536,21.130644,
	105.909638,21.139969,
	105.911496,21.146374,
	105.91437,21.168263,
	105.930018,21.187545,
	105.931896,21.192583,
	105.932212,21.219803,
	105.924032,21.244994,
	105.918095,21.251874,
	105.888306,21.280619,
	105.87834,21.324163,
	105.879788,21.342705,
	105.890848,21.36489,
	105.89178,21.370073,
	105.888131,21.38377,
	105.889869,21.397023,
	105.890429,21.427068,
	105.885788,21.447213,
	105.885703,21.474697,
	105.8832014,21.5033689,
	105.861427,21.512764,
	105.828508,21.54792,
	105.826179,21.559299,
	105.822099,21.569627,
	105.820201,21.568513
};

	//Data Area 10: Cao toc Ho Chi Minh - Dong Nai
const float Area_10[96]= {
	150, 47,
	106.7589857,10.794005,
	106.7694242,10.7954791,
	106.7922573,10.795472,
	106.8458885,10.7853933,
	106.886664,10.785631,
	106.902716,10.78297,
	106.938875,10.768518,
	106.955623,10.764416,
	106.965137,10.764494,
	107.003559,10.772777,
	107.0126883,10.7769343,
	107.021303,10.782609,
	107.0360493,10.7912872,
	107.049509,10.80085,
	107.070203,10.819909,
	107.081157,10.830053,
	107.1075733,10.8509277,
	107.130291,10.878982,
	107.144275,10.889062,
	107.152012,10.901667,
	107.162241,10.910059,
	107.164333,10.913976,
	107.16638,10.927193,
	107.16646,10.931855,
	107.16426,10.931865,
	107.16418,10.927247,
	107.162187,10.914464,
	107.160499,10.911401,
	107.150648,10.903393,
	107.147326,10.899139,
	107.142625,10.890518,
	107.128889,10.880678,
	107.10373,10.851241,
	107.0790822,10.8319399,
	107.0677729,10.8216996,
	107.042046,10.798039,
	107.002801,10.774843,
	106.964783,10.766666,
	106.957012,10.766479,
	106.939445,10.770642,
	106.903304,10.78509,
	106.886756,10.787829,
	106.84613,10.787244,
	106.792734,10.797096,
	106.7690638,10.7967517,
	106.7586343,10.7951193,
	106.7589857,10.794005
};

	//Data Area 11: Cao toc Ho Chi Minh - Trung Luong
const float Area_11[98]= {
	150, 48,
	106.320415,10.4276,
	106.3107119,10.4379769,
	106.322108,10.45388,
	106.33016,10.4714002,
	106.3364963,10.4902706,
	106.339751,10.500121,
	106.3510351,10.517445,
	106.3615479,10.5354437,
	106.3653129,10.5412022,
	106.370666,10.547467,
	106.382026,10.561256,
	106.390695,10.571671,
	106.396556,10.5777708,
	106.4104,10.592645,
	106.4483603,10.6308153,
	106.4593159,10.6391592,
	106.471431,10.647798,
	106.480203,10.6541082,
	106.4986103,10.6601765,
	106.521106,10.665186,
	106.5281977,10.6678879,
	106.5522181,10.6793774,
	106.5586622,10.681623,
	106.568304,10.6838774,
	106.5793588,10.6861449,
	106.5903924,10.6882012,
	106.5901246,10.689697,
	106.579133,10.6874035,
	106.56812,10.6852783,
	106.5584315,10.6828614,
	106.5517445,10.6806361,
	106.5275392,10.6695505,
	106.520474,10.667294,
	106.498666,10.66217,
	106.4795055,10.6556095,
	106.470089,10.649542,
	106.447091,10.63222,
	106.410165,10.5956,
	106.403127,10.587407,
	106.389065,10.573149,
	106.376693,10.558272,
	106.361029,10.539275,
	106.337869,10.501259,
	106.335565,10.495303,
	106.3201778,10.4538526,
	106.3088786,10.4381998,
	106.318805,10.4261,
	106.320415,10.4276
};

	//Data Area 12: Cao toc Lam Dong - Duc Trong
const float Area_12[68]= {
	150, 33,
	108.3845736,11.75609,
	108.3893919,11.76369,
	108.4019171,11.781709,
	108.407804,11.785991,
	108.4118922,11.791393,
	108.41795,11.793215,
	108.427205,11.799674,
	108.43297,11.810915,
	108.4346649,11.818356,
	108.4424132,11.824927,
	108.45101,11.832013,
	108.454055,11.839098,
	108.459529,11.861461,
	108.461224,11.863754,
	108.465826,11.868152,
	108.46685,11.870911,
	108.46491,11.871949,
	108.463668,11.868859,
	108.458964,11.864696,
	108.457471,11.862239,
	108.451905,11.839562,
	108.4493686,11.833183,
	108.446518,11.830331,
	108.433103,11.81896,
	108.4311504,11.811211,
	108.4251684,11.800162,
	108.4171377,11.794835,
	108.41071,11.792841,
	108.4060891,11.7868725,
	108.400642,11.782952,
	108.3911888,11.769062,
	108.382726,11.756436,
	108.3845736,11.75609
};

	//Data Area 13: Cao toc Trung Luong - My Thuan
const float Area_13[56]= {
	150, 27,
	106.3038826,10.437655,
	106.2803005,10.4353232,
	106.2558495,10.4389528,
	106.2114644,10.427916,
	106.1661458,10.4234526,
	106.1193143,10.4262805,
	106.0855829,10.4243811,
	106.0265958,10.4141353,
	105.9245538,10.3587002,
	105.8878612,10.3292743,
	105.879364,10.3204502,
	105.8817672,10.3143281,
	105.8857583,10.314877,
	105.8853292,10.323659,
	105.9267425,10.3573915,
	106.0275506,10.410959,
	106.0570115,10.4159621,
	106.0866451,10.4226507,
	106.1137676,10.4234948,
	106.167755,10.4208358,
	106.2108421,10.425774,
	106.2560105,10.4369797,
	106.2784553,10.433561,
	106.3053631,10.435397,
	106.3112854,10.4383092,
	106.3093328,10.4396386,
	106.3038826,10.437655
};

	//Data Area 14: Cao toc Ha Long - Hai Phong
const float Area_14[124]= {
	150, 61,
	106.7524897,20.8366983,
	106.7847874,20.8610487,
	106.8288642,20.9037991,
	106.8478646,20.9222338,
	106.8750346,20.9361097,
	106.8983465,20.9506481,
	106.9277481,20.9533749,
	106.9375315,20.9552916,
	106.9439618,20.9599331,
	106.9519429,20.9677159,
	106.9613187,20.9678162,
	106.9640907,20.9689068,
	106.9703726,20.9844791,
	106.9805036,20.9971353,
	106.9817502,20.9998416,
	106.9854361,21.0172103,
	106.9842819,21.0217757,
	106.9795146,21.0297356,
	106.9936662,21.0433819,
	106.9966141,21.0467162,
	106.9982661,21.0533083,
	107.0064361,21.059844,
	107.0244796,21.0594994,
	107.0497019,21.0522584,
	107.0767779,21.0520219,
	107.0896465,21.0441674,
	107.0932938,21.0433659,
	107.1208718,21.0494051,
	107.1329723,21.0493851,
	107.1482868,21.04555,
	107.1553368,21.0488201,
	107.1699475,21.0522463,
	107.1692181,21.0544103,
	107.1534574,21.0492489,
	107.1403056,21.0503108,
	107.1280316,21.0525131,
	107.1130819,21.050822,
	107.0931934,21.0444266,
	107.0774284,21.0528061,
	107.071936,21.0535875,
	107.0520593,21.0530728,
	107.0320137,21.0586374,
	107.0220587,21.0608614,
	107.0115758,21.0608894,
	107.0040008,21.0602689,
	106.998444,21.0561816,
	106.9934536,21.0444641,
	106.9772867,21.0312504,
	106.9797111,21.025499,
	106.980677,20.999622,
	106.9692618,20.9849775,
	106.9621388,20.9690364,
	106.9523983,20.9690164,
	106.9497078,20.968219,
	106.9399115,20.959117,
	106.9307632,20.95544,
	106.8986582,20.9527814,
	106.8501661,20.9263493,
	106.8021701,20.8837099,
	106.7516165,20.8382925,
	106.7524897,20.8366983
};

	//Data Area 15: Duong vao san bay Van Don
const float Area_15[124]= {
	150, 61,
	107.1701777,21.051717,
	107.1752957,21.052651,
	107.1813227,21.051287,
	107.1839017,21.0513,
	107.1903647,21.053606,
	107.2003357,21.054306,
	107.2131537,21.060694,
	107.2249587,21.062422,
	107.2475917,21.063434,
	107.2637677,21.070141,
	107.2733417,21.069082,
	107.2783007,21.073635,
	107.2874927,21.075324,
	107.2948947,21.074098,
	107.3001137,21.074838,
	107.3106967,21.080586,
	107.3160847,21.087443,
	107.3218347,21.089984,
	107.3249627,21.089653,
	107.3343437,21.083925,
	107.3462107,21.084441,
	107.3501937,21.086967,
	107.3573547,21.093768,
	107.3610887,21.093197,
	107.3826487,21.08226,
	107.3877827,21.080835,
	107.4003297,21.082101,
	107.4047567,21.084169,
	107.4072917,21.087169,
	107.4118527,21.084659,
	107.4164397,21.079114,
	107.4249037,21.076132,
	107.4266537,21.077465,
	107.4178547,21.080799,
	107.4134087,21.086214,
	107.4069827,21.089325,
	107.3996757,21.084202,
	107.3876217,21.083029,
	107.3623357,21.09501,
	107.3572637,21.095966,
	107.3547927,21.094862,
	107.3453407,21.086462,
	107.3345777,21.086112,
	107.3261417,21.09151,
	107.3217967,21.092183,
	107.3149227,21.089311,
	107.3090347,21.082027,
	107.2991347,21.076808,
	107.2875787,21.077522,
	107.2775837,21.075715,
	107.2722267,21.070978,
	107.2638467,21.072339,
	107.2467867,21.065482,
	107.2248757,21.064621,
	107.2126817,21.062843,
	107.2017307,21.057032,
	107.1900537,21.055784,
	107.1832657,21.053406,
	107.1757187,21.054809,
	107.1696767,21.053859,
	107.1701777,21.051717
};

	//Data Area 16: Lach Huyen
const float Area_16[40]= {
	150, 19,
	106.7555215,20.8390768,
	106.757967,20.8227496,
	106.7779644,20.805704,
	106.7557571,20.8031152,
	106.7519381,20.8046405,
	106.7525174,20.8004659,
	106.7573445,20.8021389,
	106.7785236,20.8042537,
	106.8893499,20.8076804,
	106.8944347,20.8060547,
	106.8986828,20.8017799,
	106.8994766,20.8022415,
	106.896387,20.8067766,
	106.8916825,20.8086512,
	106.8869196,20.8086515,
	106.7799907,20.8059089,
	106.7587675,20.8247886,
	106.7562444,20.8392682,
	106.7555215,20.8390768
};

const float * bufNumberArea[TOTOL_AREA] = {Area_1, Area_2, Area_3, Area_4, Area_5, Area_6, Area_7, Area_8, Area_9, Area_10, Area_11, Area_12, Area_13, Area_14, Area_15, Area_16};


GPS_typedef GPS_Data;

static u16 camera_count_ = 0;

mcu_type_def MCU_Data;

u8 GPS_READY = 0;

static enum {
	VEHICLE_STATE_RUN,
	VEHICLE_STATE_STOP
} vehicle_status;



// Ð?nh nghia bán kính trái d?t (theo don v? mét)
#define EARTH_RADIUS 6371000.0

// Hàm chuy?n d? sang radian
double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

// Hàm Haversine tính kho?ng cách gi?a hai t?a d? GPS
double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Chuy?n d?i t?a d? t? d? sang radian
    lat1 = deg2rad(lat1);
    lon1 = deg2rad(lon1);
    lat2 = deg2rad(lat2);
    lon2 = deg2rad(lon2);

    // Tính toán các thành ph?n c?a công th?c Haversine
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1) * cos(lat2) *
               sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    // Tính kho?ng cách
    double distance = EARTH_RADIUS * c;
    return distance;
}


//#define pi 3.14159265358979323846
//
//double deg2rad(double deg) {
//  return (deg * pi / 180);
//}
//
//double rad2deg(double rad) {
//  return (rad * 180 / pi);
//}

u16 get_frq_reset(void){
	if(DeviceSetting.time_reset_increment > 25200){
		DeviceSetting.time_reset_increment = 0;
		write_number(TIME_RESET_INCREMENT, (u64)DeviceSetting.time_reset_increment);
		sync();
	}
	if(DeviceSetting.time_reset_increment >= 3600) return DeviceSetting.time_reset_increment;
	return SystemSetting.data.time_reset_when_disconnect + DeviceSetting.time_reset_increment;
}

void increase_frq_reset(void){
	DeviceSetting.time_reset_increment = DeviceSetting.time_reset_increment + 3600;
	write_number(TIME_RESET_INCREMENT, (u64)DeviceSetting.time_reset_increment);
	sync();
}

void clear_time_reset_increment(void){
	DeviceSetting.time_reset_increment = 0;
	write_number(TIME_RESET_INCREMENT, (u64)DeviceSetting.time_reset_increment);
	sync();
}

//double calculate_distance(loc_def loc1, loc_def loc2, char unit) {
//  double theta, dist;
//  if ((loc1.latitude == loc2.latitude) && (loc1.longitude == loc2.longitude)) {
//    return 0;
//  }else {
//    theta = loc1.longitude - loc2.longitude;
//    dist = sin(deg2rad(loc1.latitude)) * sin(deg2rad(loc2.latitude)) + cos(deg2rad(loc1.latitude)) * cos(deg2rad(loc2.latitude)) * cos(deg2rad(theta));
//    dist = acos(dist);
//    dist = rad2deg(dist);
//    dist = dist * (float)60.0 * (float)1.1515;
//    switch(unit) {
//      case 'M':
//        break;
//      case 'K':
//        dist = dist * (float)1.609344;
//        break;
//      case 'm':
//        dist = dist * (float)1.609344 * (float)1000.0;
//        break;
//      case 'N':
//        dist = dist * (float)0.8684;
//        break;
//      case 'H':
//        dist = dist * (float)1.609344 * (float)0.539957;
//        break;
//    }
//    return (dist);
//  }
//}

u8 get_acc(void){
	switch(SystemSetting.data.AccMode){
		case ACC_LSM:
			return (MCU_Data.Hard_Acc | LSM_ACC);
		case ACC_ONLY:
			 return MCU_Data.Hard_Acc;
		case LSM_ONLY:
			return LSM_ACC;
		case NO_ACC_LSM_SPEED:{
			if(GPS_Data.tmp_speed > SystemSetting.data.RunLevel){
				return 1;
			}else{
				return 0;
			}
		}
		case NO_ACC_LSM_NO_SPEED:
			return 1;
	}
}

#define LOW_EXT_POWER_LEVEL  7000
u8 power_main_status(void){
	if(MCU_Data.main_power < LOW_EXT_POWER_LEVEL)
		return 0;
	return 1;
}
#define POWER_STATUS power_main_status()

static void cal_tcp_freq(void){
	if(Vehicle_in_China() == true){
		if(ACC_STATUS){
			GPS_Data.freq_send = (SystemSetting.data.FrqRun * 6);
		}else{
			GPS_Data.freq_send = (SystemSetting.data.FrqStop * 3);
		}
	}else{
		if(ACC_STATUS){
			GPS_Data.acc_time++;
			if(GPS_Data.acc_time > 10){
				GPS_Data.acc_time = 0;
				GPS_Data.stop_time = 0;
			}
		}else if(GPS_Data.stop_time < TIME_10_MINUTES){
			GPS_Data.stop_time ++;
		}

		if(GPS_Data.stop_time > TIME_3_MINUTES){
			vehicle_status = VEHICLE_STATE_STOP;
			GPS_Data.freq_send = SystemSetting.data.FrqStop;
			GPS_Data.freq_send_camera = SystemSetting.data.FrqCaptureStop;
		}else{
			vehicle_status = VEHICLE_STATE_RUN;
			GPS_Data.freq_send = SystemSetting.data.FrqRun;
			GPS_Data.freq_send_camera = SystemSetting.data.FrqCaptureRun;
		}
	}
}

void update_rfid_driver_data(void){
	u8 tmp8;
	u16 tmp16;
	u64 tmp64;
	u8 mcu_data[256];

	mcu_data[0] = '>';
	mcu_data[1] = '>';
	mcu_data[2] = 0x03;

	/* device status */
	mcu_data[3] = tibase.LoginStatus | (tibase.driver_info.sync << 4);
	mcu_data[4] = tibase.UID & 0xFF;
	mcu_data[5] = (tibase.UID >> 8)  & 0xFF;
	mcu_data[6] = (tibase.UID >> 16) & 0xFF;
	mcu_data[7] = (tibase.UID >> 24) & 0xFF;
	mcu_data[8] = (tibase.UID >> 32) & 0xFF;
	mcu_data[9] = (tibase.UID >> 40) & 0xFF;
	mcu_data[10] =  (tibase.UID >> 48) & 0xFF;

	/* is sync tag uid */
	mcu_data[9] = tibase.driver_info.sync;

	memcpy(mcu_data + 11, tibase.driver_info.DriverName, MAX_SIZE_DRIVER_NAME);

	memcpy(mcu_data + 11 + MAX_SIZE_DRIVER_NAME, tibase.driver_info.DriverLicense, MAX_SIZE_DRIVER_LICENSE);

	memcpy(mcu_data + 11 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE, tibase.driver_info.IssDate, MAX_SIZE_TAG_DATE);

	memcpy(mcu_data + 11 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE, tibase.driver_info.ExpDate, MAX_SIZE_TAG_DATE);

	//CRC
	tmp8 = CRC_8BitsCompute(mcu_data, (11 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE));
	mcu_data[11 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE] = tmp8;
	rfid_send_msg(mcu_data, 1 + 11 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE + MAX_SIZE_TAG_DATE);

	if(GPS_READY == 1){
		cam_setting_driver_info(tibase.driver_info.DriverName, tibase.driver_info.DriverLicense);
	}
}

static void update_rfid_status(void){
	u8 tmp8;
	u16 tmp16;
	u8 mcu_data[128];
	mcu_data[0] = '>';   //0x1
	mcu_data[1] = '>';
	mcu_data[2] = 0x01;

	//log_gps("UPDATE -> STATUS RFID DATA \n");

	/* device status */
	mcu_data[3] = GPS_Data.gps_status | ((u8)network_data.tcp_status << 1) | ((u8)network_data.internet_status << 2) | (ACC_STATUS << 3) | (((tibase.LoginStatus == STATUS_LOGIN) ? 1 : 0) << 4) | ((u8)tibase.over_speed_status << 5) | ((u8)tibase.over_driver_continue_status << 6) | (((network_data.sim_status == E_QL_SIM_STAT_READY) ? 1 : 0) << 7);
	/* buzzer level */
	mcu_data[4] = SystemSetting.data.BuzzerLevel;
	/* slience mode */
	mcu_data[5] = SystemSetting.data.SlienceMode;
	//CRC
	tmp8 = CRC_8BitsCompute(mcu_data, 6);
	mcu_data[6] = tmp8;
	rfid_send_msg(mcu_data, 7);
	//log_system("UPDATE RFID DATA\n");
}

void update_mcu_gps_data(void){
	u8 tmp8;
	u16 tmp16;
	u8 mcu_data[128];
	mcu_data[0] = '>';   //0x1
	mcu_data[1] = '>';
	mcu_data[2] = 0x01;

	log_gps("UPDATE -> GPS MCU DATA \n");

	/* device status */
	mcu_data[3] = GPS_Data.gps_status | ((u8)network_data.tcp_status << 1) | ((u8)network_data.internet_status << 2) | (((tibase.LoginStatus == STATUS_LOGIN) ? 1 : 0) << 3) | ((u8)tibase.over_speed_status << 4) | ((u8)tibase.over_driver_continue_status << 5);

	/* sim status */
	mcu_data[4] = network_data.sim_status;

	/* obd type */
	mcu_data[5] = SystemSetting.data.OBDType;

	/* adc pull config */
	mcu_data[6] = SystemSetting.data.AdcPulseCfg;

	/* buzzer level */
	mcu_data[7] = SystemSetting.data.BuzzerLevel;

	/* slience mode */
	mcu_data[8] = SystemSetting.data.SlienceMode;

//	mcu_data[9] = (DeviceSetting.FuelManualMode >> 8) & 0xFF;
//	mcu_data[10] =  DeviceSetting.FuelManualMode & 0xFF;

	mcu_data[9] = (tibase.over_driver_inday_status) | (tibase.over_driver_inweek_status << 1) | ((u8)network_data.is_cam_warning << 2) | ((u8)network_data.move_logout_warning << 3);
	mcu_data[10] = 0xFF;

	mcu_data[11] = (DeviceSetting.BauRate2 >> 24) & 0xFF;
	mcu_data[12] = (DeviceSetting.BauRate2 >> 16) & 0xFF;
	mcu_data[13] = (DeviceSetting.BauRate2 >> 8) & 0xFF;
	mcu_data[14] = DeviceSetting.BauRate2 & 0xFF;

	mcu_data[15] = (DeviceSetting.BauRate4 >> 24) & 0xFF;
	mcu_data[16] = (DeviceSetting.BauRate4 >> 16) & 0xFF;
	mcu_data[17] = (DeviceSetting.BauRate4 >> 8) & 0xFF;
	mcu_data[18] = DeviceSetting.BauRate4 & 0xFF;

	//CRC
	tmp8 = CRC_8BitsCompute(mcu_data, 19);
	mcu_data[19] = tmp8;
	mcu_send_msg(mcu_data, 20);
}

void mcu_report_data_parser(u8 * data){
	u8 tmp0;
	int tmp1, tmp2;
	MCU_Data.MCU_Verison = data[0];

	MCU_Data.main_power = (data[1] << 8) | data[2];

	MCU_Data.bat_power = (data[3] << 8) | data[4];

	MCU_Data.fuel_adc_in = (data[5] << 8) | data[6];

	MCU_Data.Vout_power = (data[7] << 8) | data[8];

	tmp1 = (data[9] << 8) | data[10]; //	MCU_Data.fuel_232_1 = (data[9] << 8) | data[10];
	tmp2 = (data[11] << 8) | data[12];//	MCU_Data.fuel_232_2 = (data[11] << 8) | data[12];

	if(SystemSetting.data.fuel_3_to_1 == 1){
		tmp1 = MCU_Data.fuel_adc_in;
		MCU_Data.fuel_adc_in = 0;
	}else if(SystemSetting.data.fuel_2_to_1 == 1){
		tmp1 = tmp2;
		tmp2 = 0;
	}

	if(SystemSetting.data.fuel_3_to_2 == 1){
		tmp2 = MCU_Data.fuel_adc_in;
		MCU_Data.fuel_adc_in = 0;
	}

	///mat nguon lay gia tri truoc do lam gia tri hien tai
	if(MCU_Data.main_power < LOW_EXT_POWER_LEVEL){
		tmp1 = MCU_Data.fuel_232_1;
		tmp2 = MCU_Data.fuel_232_2;
	}

	/* filter ad value */
	add_fuel_data(tmp1, tmp2);

	GPS_Data.pulse_speed = data[13];

	//tibase.temperature = (data[14] << 8) | data[15];   //16    --    17    no data

	MCU_Data.obd_data.connected = data[18] & 0x01;

	MCU_Data.Hard_Acc = ((data[18] >> 1) & 0x01);

	tmp0 = ((data[18] >> 2) & 0x01);

	MCU_Data.Hard_Door = ((SystemSetting.data.DoorRevert == 1) ? !tmp0 : tmp0);

	tmp0 = ((data[18] >> 3) & 0x01);

	MCU_Data.Hard_Air = ((SystemSetting.data.AirRevert == 1) ? !tmp0 : tmp0);

	MCU_Data.power_mode = (power_type)((data[18] >> 4) & 0x01);

	MCU_Data.obd_data.typeOBD = data[19];

	MCU_Data.obd_data.sp = data[20];

	MCU_Data.obd_data.temp = data[21];

	MCU_Data.obd_data.rpm = (data[22] << 8) | data[23];

	MCU_Data.obd_data.p_fuel = (data[24] << 8) | data[25];

	MCU_Data.obd_data.s_km = (data[26] << 24) | (data[27] << 16) | (data[28] << 8) | data[29];

	MCU_Data.obd_data.s_fuel = (data[30] << 24) | (data[31] << 16) | (data[32] << 8) | data[33];

	MCU_Data.obd_data.bat = (data[34] << 8) | data[35];

	MCU_Data.obd_data.avg_econ_fuel = (data[36] << 8) | data[37];

	MCU_Data.obd_data.econ_fuel = (data[38] << 8) | data[39];

	MCU_Data.obd_data.p_load = data[40];

//	log_system("SPEED -> %d, temp -> %d, RPM -> %d, Pfuel -> %d, s_km -> %u, s_fuel -> %d , bat -> %d, avg_econ_fuel-> %d, p_load -> %d",
//	MCU_Data.obd_data.sp, MCU_Data.obd_data.temp, MCU_Data.obd_data.rpm, MCU_Data.obd_data.p_fuel, MCU_Data.obd_data.s_km, MCU_Data.obd_data.s_fuel, MCU_Data.obd_data.bat, MCU_Data.obd_data.avg_econ_fuel, MCU_Data.obd_data.p_load);

	//log_gps("ACC %d - DOOR %d - AIR %d \n MCU VER : %d \n", MCU_Data.Hard_Acc, MCU_Data.Hard_Door, MCU_Data.Hard_Air, MCU_Data.MCU_Verison);
	if((MCU_Data.power_mode == POWER_SAVE) && (MCU_Data.last_power_mode == POWER_BALANCE)){
		GPS_Data.stop_time = 0;

		if(MCU_Data.pwr_warning_delay_tm > 60){
			MCU_Data.pwr_warning_delay_tm = 0;
			if(SystemSetting.data.Phone_sos[0] == '0' && strlen(SystemSetting.data.Phone_sos) == 10){
				char pw_warning[256];

				if(GPS_Data.gps_status == 0){
					sprintf(pw_warning, "Canh bao! Xe %s Mat nguon, Mat tin hieu GPS\0", DeviceSetting.NumberPlate);
				}else{
					sprintf(pw_warning, "Canh bao! Xe %s Mat nguon!!!\n https://www.google.com/maps?q=%.5f,%.5f\nTime: %02d:%02d:%02d %02d-%02d-%d\nSpeed: %.2f\0", DeviceSetting.NumberPlate,
					GPS_Data.location.latitude, GPS_Data.location.longitude,GPS_Data.gps_time->tm_hour, GPS_Data.gps_time->tm_min, GPS_Data.gps_time->tm_sec, GPS_Data.gps_time->tm_mday, (GPS_Data.gps_time->tm_mon + 1), (GPS_Data.gps_time->tm_year + 1900), GPS_Data.gps_speed);
				}

				Send_SMS(SystemSetting.data.Phone_sos, pw_warning);
			}
		}
	}

	if(MCU_Data.Hard_Acc == 1){
		if(MCU_Data.last_Hard_Acc == 0){
			send_device_setting();
		}

		GPS_Data.no_acc_time = 0;
	}

	if((MCU_Data.Hard_Door == 1) && (MCU_Data.Last_Hard_Door == 0)){
		send_realtime_data();
	}
	MCU_Data.Last_Hard_Door = MCU_Data.Hard_Door;

	MCU_Data.last_Hard_Acc = MCU_Data.Hard_Acc;
	MCU_Data.last_power_mode = MCU_Data.power_mode;
}

int write_gps_data(void){
	trip_data_typedef GPS;
	memset(&GPS, 0x0, sizeof(GPS));
	u8 sim_status = 0;
	if(E_QL_SIM_STAT_READY == network_data.sim_status){
		sim_status = 1;
	}

	GPS.d_time = GPS_Data.gps_time;
	GPS.status_1 = GPS_Data.gps_status | (ACC_STATUS << 1) | (MCU_Data.Hard_Door << 2) | (MCU_Data.Hard_Air << 3) | (POWER_STATUS << 4) | (((tibase.LoginStatus == STATUS_LOGIN) ? 1 : 0) << 5) |((u8)tibase.over_speed_status << 6) | ((u8)tibase.over_driver_continue_status << 7);
	GPS.status_2 = network_data.net_work_info.radio_tech | (MCU_Data.Hard_Acc << 4) | ((u8)LSM_Status << 5) | (sim_status << 7);
	GPS.tm_stamp = (u32)GPS_Data.timestamp;
	GPS.location.latitude = GPS_Data.location.latitude;
	GPS.location.longitude = GPS_Data.location.longitude;
	GPS.angle = (u16)GPS_Data.bearing;
	GPS.pulse_speed = GPS_Data.svs_in_use | ((u8)network_data.tcp_status << 7);
	GPS.speed_limit = speed_limit_device;
	GPS.gps_distance = (tibase.GPSMeterInday / 10);
	GPS.DriveInWeek = (tibase.DriverInWeek / 60);
	GPS.fuel1 = MCU_Data.fuel_232_1;
	GPS.fuel2 = MCU_Data.fuel_232_2;
	GPS.fuel3 = MCU_Data.fuel_adc_in;
	GPS.tagUID = tibase.UID;
	GPS.GSMStreng = network_data.net_work_info.signal_strength;
	GPS.main_power = MCU_Data.main_power;
	GPS.batery_power = MCU_Data.bat_power;
	GPS.frqsend = GPS_Data.freq_send;
	GPS.speed1 = GPS_Data.speeds[0];
	GPS.speed2 = GPS_Data.speeds[1];
	GPS.speed3 = GPS_Data.speeds[2];
	GPS.speed4 = GPS_Data.speeds[3];
	GPS.speed5 = GPS_Data.speeds[4];
	GPS.speed6 = GPS_Data.speeds[5];
	GPS.speed7 = GPS_Data.speeds[6];
	GPS.speed8 = GPS_Data.speeds[7];
	GPS.speed9 = GPS_Data.speeds[8];
	GPS.speed10 = GPS_Data.speeds[9];
	GPS.FlgSend = 0;

	return insert_TripInfo(GPS);
}

void compress_locations(u8* output, loc_def *locs, u8 nLoc){
	u8 i = 0;
	float deltalat = 0;
	float deltalong = 0;
	s16 dlt,dln;

	for(i = (nLoc - 1); i > 0; i--){
		deltalat = locs[i].latitude  - locs[i - 1].latitude;
		deltalong = locs[i].longitude - locs[i - 1].longitude;

		if(deltalat > 0.032000) deltalat = 0.0;
		if(deltalong > 0.032000) deltalong = 0.0;

		dlt = (s16)(deltalat * 1000000);
		dln = (s16)(deltalong * 1000000);

		output[(i * 4) - 1] = (u8)((dlt >> 8) & 0xFF);
		output[(i * 4) - 2] = (u8)(dlt & 0xFF);

		output[(i * 4) - 3] = (u8)((dln >> 8) & 0xFF);
		output[(i * 4) - 4] = (u8)(dln & 0xFF);
	}
}

void decompress_locations(u8* input, loc_def *locs, loc_def end_point, u8 nLoc){
	u8 i = 0;
	s16 dlt,dln;
	loc_def this_point = end_point;
	loc_def tmp;

	for(i = (nLoc - 1); i > 0; i--){
		dlt = (s16)((input[(i * 4) - 1] << 8) | (input[(i * 4) - 2]));
		tmp.latitude = this_point.latitude - ((double)dlt / (double)1000000);


		dln = (s16)((input[(i * 4) - 3] << 8) | (input[(i * 4) - 4]));
		tmp.longitude = this_point.longitude - ((double)dln / (double)1000000);

		//log_gps("DECOMPRESS -> %d    %f, %f \n", i, tmp.latitude, tmp.longitude);

		locs[i - 1] = tmp;
		this_point = tmp;
	}
}

static void SendOverSpeed(void){
	tcp_mail_queue_typedef buf;
	u16 tcp_len;
	u8 crc = 0;

	/* Frame header */
	buf.data[0] = 0xF0;
	buf.data[1] = 0xF0;
	/* Device ID */
	buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
	buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
	buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
	buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
	buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
	buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
	buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
	buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
	/* Version */
	buf.data[10] = OVER_SPEED;

	/* Average speed  */
	buf.data[13] = (GPS_Data.ovSpeedData.speeds / (GPS_Data.ovSpeedData.ovSpeedHold + 1));

	/* Date Time*/
	*(int32_t*)(buf.data + 14) = GPS_Data.timestamp;
	/* Latitude */
	*(float*)(buf.data + 18) = GPS_Data.location.latitude;
	/* longitude */
	*(float*)(buf.data + 22) = GPS_Data.location.longitude;

	buf.data[26] = tibase.UID & 0xFF;
	buf.data[27] = (tibase.UID >> 8)  & 0xFF;
	buf.data[28] = (tibase.UID >> 16) & 0xFF;
	buf.data[29] = (tibase.UID >> 24) & 0xFF;
	buf.data[30] = (tibase.UID >> 32) & 0xFF;
	buf.data[31] = (tibase.UID >> 40) & 0xFF;
	buf.data[32] = (tibase.UID >> 48) & 0xFF;
	buf.data[33] = (tibase.UID >> 56) & 0xFF;

	/* Length data */
	tcp_len = 35;
	buf.data[11] = tcp_len & 0xFF;
	buf.data[12] = tcp_len >> 8;

	/* CRC */
	crc = CRC_8BitsCompute(buf.data, 34);
	buf.data[34] = crc;

	buf.size = tcp_len;
	buf.Data_Type = OVER_SPEED;
	buf.trace = GPS_Data.timestamp;

	Push_TCP_Data(buf);
}

#if (USE_ENCRYPT == 1)
	u8 data_raw[256];
	void send_realtime_data(void){
		u8 idx = 0;
		tcp_mail_queue_typedef buf;

		s16 tmps16 = 0;
		u8 crc = 0;
		u16 tmp16 = 0;
		u32 tmp32 = 0;
		u64 tmp64 = 0;
		u16 tcp_len = 136;
		u8 svr_status = (u8)network_data.tcp_status;

		/* Frame header */
		data_raw[0] = 0xF1;
		data_raw[1] = 0xF1;

		/* Device ID */
		data_raw[2] =  SystemSetting.data.DeviceID & 0xFF;
		data_raw[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
		data_raw[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
		data_raw[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
		data_raw[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
		data_raw[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
		data_raw[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
		data_raw[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;

		/* Date Time*/
		*(int32_t*)(data_raw + 10) = GPS_Data.timestamp;

		data_raw[14] = tcp_len & 0xFF;
		data_raw[15] = tcp_len >> 8;

		/* Version */
		data_raw[16] = REAL_TIME_DATA;
		/* Status of device  GPS - ACC - DOOR - AIR  - power mode (0 - nomal, 1 low pw) - LOGIN - OVSPEED - OV-LXLT */
		data_raw[17] = GPS_Data.gps_status | (ACC_STATUS << 1) | (MCU_Data.Hard_Door << 2) | (MCU_Data.Hard_Air << 3) | (POWER_STATUS << 4) | (((tibase.LoginStatus == STATUS_LOGIN) ? 1 : 0) << 5) |((u8)tibase.over_speed_status << 6) | ((u8)tibase.over_driver_continue_status << 7);
		data_raw[18] = (u8)network_data.net_work_info.radio_tech | (MCU_Data.Hard_Acc << 4) | ((u8)LSM_Status << 5);
		/* Latitude */
		*(float*)(data_raw + 19) = GPS_Data.location.latitude;
		/* longitude */
		*(float*)(data_raw + 23) = GPS_Data.location.longitude;
		/* Course */
		tmp16 = (u16)GPS_Data.bearing;
		data_raw[27] =  tmp16 & 0xFF;
		data_raw[28] = tmp16 >> 8;
		/* Speed */
		data_raw[29] = (u8)(GPS_Data.gps_speed);
		/* gps meter */
		tmp16 = (tibase.GPSMeterInday / 100);
		data_raw[30] = tmp16 & 0xFF;
		data_raw[31] = tmp16 >> 8;

		/* pulse speed */
		data_raw[32] = GPS_Data.svs_in_use | ((u8)network_data.tcp_status << 7);
		/* temp */
		data_raw[33] = 0;//tibase.temperature & 0xFF;
		data_raw[34] = 0;//tibase.temperature >> 8;

		/* main power value */
		data_raw[35] = MCU_Data.main_power & 0xFF;
		data_raw[36] = MCU_Data.main_power >> 8;

		/* battery power value */
		data_raw[37] = MCU_Data.bat_power & 0xFF;
		data_raw[38] = MCU_Data.bat_power >> 8;

		/* max speed */
		data_raw[39] = speed_limit_device;
		/* Signal Strength */
		data_raw[40] = network_data.net_work_info.signal_strength;
		/* Fuel 1 */
		data_raw[41] = MCU_Data.fuel_232_1 & 0xFF;
		data_raw[42] = MCU_Data.fuel_232_1 >> 8;
		/* Fuel 2 */
		data_raw[43] = MCU_Data.fuel_232_2 & 0xFF;
		data_raw[44] = MCU_Data.fuel_232_2 >> 8;
		/* Fuel 3 */
		data_raw[45] = MCU_Data.fuel_adc_in & 0xFF;
		data_raw[46] = MCU_Data.fuel_adc_in >> 8;

		/* ID of rfid card */
		tmp64 = tibase.UID;
		data_raw[47] = tmp64 & 0xFF;
		data_raw[48] = (tmp64 >> 8)  & 0xFF;
		data_raw[49] = (tmp64 >> 16) & 0xFF;
		data_raw[50] = (tmp64 >> 24) & 0xFF;
		data_raw[51] = (tmp64 >> 32) & 0xFF;
		data_raw[52] = (tmp64 >> 40) & 0xFF;
		data_raw[53] = (tmp64 >> 48) & 0xFF;
		data_raw[54] = (tmp64 >> 56) & 0xFF;

		data_raw[55] = GPS_Data.freq_send & 0xFF;
		data_raw[56] = GPS_Data.freq_send >> 8;

		/* speeds over seconds */
		memcpy(data_raw + 57, (u8*)GPS_Data.speeds, SystemSetting.data.FrqRun);
		/* locations per seconds */
		compress_locations(data_raw + 67, GPS_Data.locations, SystemSetting.data.FrqRun); //LENGHT = 9 x4 = 36 byte

		/**************************************************/

		/* minute of driver continue */
		tmp16 = (get_run_time() / 60);
		data_raw[103] = tmp16 & 0xFF;
		data_raw[104] = tmp16 >> 8;

		/* minute of driver stop */
		tmp16 = (get_stop_time() / 60);
		data_raw[105] = tmp16 & 0xFF;
		data_raw[106] = tmp16 >> 8;

		/* minute of driver in day */
		tmp16 = (tibase.DriverInday / 60);
		data_raw[107] = tmp16 & 0xFF;
		data_raw[108] = tmp16 >> 8;

		/* count number of park */
		data_raw[109] = tibase.ParkCount;

		/* count number of over speed */
		data_raw[110] = tibase.OverSpeedCount;

		/* count number of over speed */
		data_raw[111] = tibase.DriveContinueCount;

		/* count number of over speed */
		data_raw[112] = tibase.OverDriveContinueCount;

		/* OBD connect */
		data_raw[113] = MCU_Data.obd_data.connected;

		/* Vehicle speed */
		data_raw[114] = MCU_Data.obd_data.sp;

		/* cooling water temperature */
		data_raw[115] = MCU_Data.obd_data.temp;

		/* engine RPM */
		data_raw[116] = MCU_Data.obd_data.rpm & 0xFF;
		data_raw[117] = MCU_Data.obd_data.rpm >> 8;

		data_raw[118] = MCU_Data.obd_data.p_fuel & 0xFF;
		data_raw[119] = MCU_Data.obd_data.p_fuel >> 8;

		/* total km */
		data_raw[120] =  MCU_Data.obd_data.s_km & 0xFF;
		data_raw[121] = (MCU_Data.obd_data.s_km >> 8) & 0xFF;
		data_raw[122] = (MCU_Data.obd_data.s_km >> 16) & 0xFF;
		data_raw[123] = (MCU_Data.obd_data.s_km >> 24) & 0xFF;

		/* total fuel */
		data_raw[124] =  MCU_Data.obd_data.s_fuel & 0xFF;
		data_raw[125] = (MCU_Data.obd_data.s_fuel >> 8) & 0xFF;
		data_raw[126] = (MCU_Data.obd_data.s_fuel >> 16) & 0xFF;
		data_raw[127] = (MCU_Data.obd_data.s_fuel >> 24) & 0xFF;

		/* accu voltage */
		data_raw[128] = MCU_Data.obd_data.bat & 0xFF;
		data_raw[129] = MCU_Data.obd_data.bat >> 8;

		/* avg econ fuel */
		data_raw[130] = MCU_Data.obd_data.avg_econ_fuel & 0xFF;
		data_raw[131] = MCU_Data.obd_data.avg_econ_fuel >> 8;

		data_raw[132] = MCU_Data.obd_data.econ_fuel & 0xFF;
		data_raw[133] = MCU_Data.obd_data.econ_fuel >> 8;

		/* power load */
		data_raw[134] = MCU_Data.obd_data.p_load;

		memcpy(buf.data, data_raw, tcp_len);

		/* encrypt data */
		encrypt_GPSdata(DeviceSetting.IMEI, GPS_Data.timestamp, (data_raw + 16), (buf.data + 16), (tcp_len - 17));

		/* CRC */
		crc = CRC_8BitsCompute(buf.data, 135);
		buf.data[135] = crc;

//		printf("TIME -> %u /n", GPS_Data.timestamp);
//		printf("RAW /n");
//		hex_log_debug(data_raw, tcp_len - 1);
//
//		printf("ENCRYPT /n");
//		hex_log_debug(buf.data, tcp_len);

		buf.size = tcp_len;
		buf.Data_Type = REAL_TIME_DATA;
		buf.trace = GPS_Data.timestamp;

		Push_TCP_Data(buf);
	}
#else
	void send_realtime_data(void){
		u8 idx = 0;
		tcp_mail_queue_typedef buf;

		s16 tmps16 = 0;
		u8 crc = 0;
		u16 tmp16 = 0;
		u32 tmp32 = 0;
		u64 tmp64 = 0;
		u16 tcp_len = 0;
		u8 sim_status = 0;
		if(E_QL_SIM_STAT_READY == network_data.sim_status){
			sim_status = 1;
		}

		/* Frame header */
		buf.data[0] = 0xF0;
		buf.data[1] = 0xF0;
		/* Device ID */
		buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
		buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
		buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
		buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
		buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
		buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
		buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
		buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
		/* Version */
		buf.data[10] = REAL_TIME_DATA;
		/* Status of device  GPS - ACC - DOOR - AIR  - power mode (0 - nomal, 1 low pw) - LOGIN - OVSPEED - OV-LXLT */
		buf.data[13] = GPS_Data.gps_status | (ACC_STATUS << 1) | (MCU_Data.Hard_Door << 2) | (MCU_Data.Hard_Air << 3) | (POWER_STATUS << 4) | (((tibase.LoginStatus == STATUS_LOGIN) ? 1 : 0) << 5) |((u8)tibase.over_speed_status << 6) | ((u8)tibase.over_driver_continue_status << 7);
		buf.data[14] = (u8)network_data.net_work_info.radio_tech | (MCU_Data.Hard_Acc << 4) | ((u8)LSM_Status << 5) | (sim_status << 7);
		/* Date Time*/
		*(int32_t*)(buf.data + 15) = GPS_Data.timestamp;
		/* Latitude */
		*(float*)(buf.data + 19) = GPS_Data.location.latitude;
		/* longitude */
		*(float*)(buf.data + 23) = GPS_Data.location.longitude;
		/* Course */
		tmp16 = (u16)GPS_Data.bearing;
		buf.data[27] =  tmp16 & 0xFF;
		buf.data[28] = tmp16 >> 8;
		/* Speed */
		buf.data[29] = (u8)(GPS_Data.gps_speed);
		/* gps meter */
		tmp16 = (tibase.GPSMeterInday / 100);
		buf.data[30] = tmp16 & 0xFF;
		buf.data[31] = tmp16 >> 8;

		/* pulse speed */
		buf.data[32] = (GPS_Data.svs_in_use & 0x1F) | (pw_camera_status << 6) | ((u8)network_data.cam_status << 7);

		/* drive in week minute */
		tmp16 = (tibase.DriverInWeek / 60);
		buf.data[33] = tmp16 & 0xFF;
		buf.data[34] = tmp16 >> 8;

		/* main power value */
		buf.data[35] = MCU_Data.main_power & 0xFF;
		buf.data[36] = MCU_Data.main_power >> 8;

		/* battery power value */
		buf.data[37] = MCU_Data.bat_power & 0xFF;
		buf.data[38] = MCU_Data.bat_power >> 8;

		/* max speed */
		buf.data[39] = speed_limit_device;
		/* Signal Strength */
		buf.data[40] = network_data.net_work_info.signal_strength;
		/* Fuel 1 */
		buf.data[41] = MCU_Data.fuel_232_1 & 0xFF;
		buf.data[42] = MCU_Data.fuel_232_1 >> 8;
		/* Fuel 2 */
		buf.data[43] = MCU_Data.fuel_232_2 & 0xFF;
		buf.data[44] = MCU_Data.fuel_232_2 >> 8;
		/* Fuel 3 */
		buf.data[45] = MCU_Data.fuel_adc_in & 0xFF;
		buf.data[46] = MCU_Data.fuel_adc_in >> 8;

		/* ID of rfid card */
		tmp64 = tibase.UID;
		buf.data[47] = tmp64 & 0xFF;
		buf.data[48] = (tmp64 >> 8)  & 0xFF;
		buf.data[49] = (tmp64 >> 16) & 0xFF;
		buf.data[50] = (tmp64 >> 24) & 0xFF;
		buf.data[51] = (tmp64 >> 32) & 0xFF;
		buf.data[52] = (tmp64 >> 40) & 0xFF;
		buf.data[53] = (tmp64 >> 48) & 0xFF;
		buf.data[54] = (tmp64 >> 56) & 0xFF;

		buf.data[55] = GPS_Data.freq_send & 0xFF;
		buf.data[56] = GPS_Data.freq_send >> 8;

		/* speeds over seconds */
		memcpy(buf.data + 57, (u8*)GPS_Data.speeds, SystemSetting.data.FrqRun);
		/* locations per seconds */
		compress_locations(buf.data + 67, GPS_Data.locations, SystemSetting.data.FrqRun); //LENGHT = 9 x4 = 36 byte

		/**************************************************/

		/* minute of driver continue */
		tmp16 = (get_run_time() / 60);
		buf.data[103] = tmp16 & 0xFF;
		buf.data[104] = tmp16 >> 8;

		/* minute of driver stop */
		tmp16 = (get_stop_time() / 60);
		buf.data[105] = tmp16 & 0xFF;
		buf.data[106] = tmp16 >> 8;

		/* minute of driver in day */
		tmp16 = (tibase.DriverInday / 60);
		buf.data[107] = tmp16 & 0xFF;
		buf.data[108] = tmp16 >> 8;

		/* count number of park */
		buf.data[109] = tibase.ParkCount;

		/* count number of over speed */
		buf.data[110] = tibase.OverSpeedCount;

		/* count number of over speed */
		buf.data[111] = tibase.DriveContinueCount;

		/* count number of over speed */
		buf.data[112] = tibase.OverDriveContinueCount;

		/* OBD connect */
		buf.data[113] = MCU_Data.obd_data.connected;

		/* Vehicle speed */
		buf.data[114] = MCU_Data.obd_data.sp;

		/* cooling water temperature */
		buf.data[115] = MCU_Data.obd_data.temp;

		/* engine RPM */
		buf.data[116] = MCU_Data.obd_data.rpm & 0xFF;
		buf.data[117] = MCU_Data.obd_data.rpm >> 8;

		buf.data[118] = MCU_Data.obd_data.p_fuel & 0xFF;
		buf.data[119] = MCU_Data.obd_data.p_fuel >> 8;

		/* total km */
		buf.data[120] =  MCU_Data.obd_data.s_km & 0xFF;
		buf.data[121] = (MCU_Data.obd_data.s_km >> 8) & 0xFF;
		buf.data[122] = (MCU_Data.obd_data.s_km >> 16) & 0xFF;
		buf.data[123] = (MCU_Data.obd_data.s_km >> 24) & 0xFF;

		/* total fuel */
		buf.data[124] =  MCU_Data.obd_data.s_fuel & 0xFF;
		buf.data[125] = (MCU_Data.obd_data.s_fuel >> 8) & 0xFF;
		buf.data[126] = (MCU_Data.obd_data.s_fuel >> 16) & 0xFF;
		buf.data[127] = (MCU_Data.obd_data.s_fuel >> 24) & 0xFF;

		/* accu voltage */
		buf.data[128] = MCU_Data.obd_data.bat & 0xFF;
		buf.data[129] = MCU_Data.obd_data.bat >> 8;

		/* avg econ fuel */
		buf.data[130] = MCU_Data.obd_data.avg_econ_fuel & 0xFF;
		buf.data[131] = MCU_Data.obd_data.avg_econ_fuel >> 8;

		buf.data[132] = MCU_Data.obd_data.econ_fuel & 0xFF;
		buf.data[133] = MCU_Data.obd_data.econ_fuel >> 8;

		/* power load */
		buf.data[134] = MCU_Data.obd_data.p_load;

		/* Length data */
		tcp_len = 136;
		buf.data[11] = tcp_len & 0xFF;
		buf.data[12] = tcp_len >> 8;

		/* CRC */
		crc = CRC_8BitsCompute(buf.data, 135);
		buf.data[135] = crc;

		buf.size = tcp_len;
		buf.Data_Type = REAL_TIME_DATA;
		buf.trace = GPS_Data.timestamp;

		Push_TCP_Data(buf);
	}
#endif /* USE_ENCRYPT */

u8 cam_sn_list[128] = {0};
void send_device_setting(void){
	u8 idx = 0;
	tcp_mail_queue_typedef buf;
	char str_motion_value[32];
	u8 crc = 0, tmp = 0;
	u16 tcp_len = 0;
	u32 tmp32 = 0;
	int rs1 = 0, rs2 = 0, rs3 = 0, rs4 = 0;

	if(camera_detect(11) == 0){
		rs1 = 1;
	}

	if(camera_detect(12) == 0){
		rs2 = 1;
	}

	if(camera_detect(13) == 0){
		rs3 = 1;
	}

	if(camera_detect(14) == 0){
		rs4 = 1;
	}

	/* Frame header */
	buf.data[0] = 0xF0;
	buf.data[1] = 0xF0;
	/* Device ID */
	buf.data[2] =  SystemSetting.data.DeviceID & 0xFF;
	buf.data[3] = (SystemSetting.data.DeviceID >> 8) & 0xFF;
	buf.data[4] = (SystemSetting.data.DeviceID >> 16) & 0xFF;
	buf.data[5] = (SystemSetting.data.DeviceID >> 24) & 0xFF;
	buf.data[6] = (SystemSetting.data.DeviceID >> 32) & 0xFF;
	buf.data[7] = (SystemSetting.data.DeviceID >> 40) & 0xFF;
	buf.data[8] = (SystemSetting.data.DeviceID >> 48) & 0xFF;
	buf.data[9] = (SystemSetting.data.DeviceID >> 56) & 0xFF;
	/* Version */
	buf.data[10] = DEVICE_SETTING;

	buf.data[13] = APP_VERSION;

	buf.data[14] = MCU_Data.MCU_Verison;

	buf.data[15] = MCU_Data.RFID_Verison;

	buf.data[16] = MCU_Data.ID_RFID & 0xFF;
	buf.data[17] = (MCU_Data.ID_RFID >> 8) & 0xFF;
	buf.data[18] = (MCU_Data.ID_RFID >> 16) & 0xFF;
	buf.data[19] = (MCU_Data.ID_RFID >> 24) & 0xFF;

	buf.data[20] = SystemSetting.data.OBDType;

	buf.data[21] = SystemSetting.data.AdcPulseCfg;

	buf.data[22] = SystemSetting.data.BuzzerLevel;

	buf.data[23] = SystemSetting.data.SpeakerVolume;

	buf.data[24] = SystemSetting.data.RunLevel;

	buf.data[25] = SystemSetting.data.AccMode;

	buf.data[26] = SystemSetting.data.DoorRevert;

	buf.data[27] = SystemSetting.data.AirRevert;

	buf.data[28] = DeviceSetting.CarType;

	/* time send gps when stop */
	buf.data[29] = SystemSetting.data.FrqRun;

	/* slience mode */
	buf.data[30] = (rs1 | (rs2 << 1) | (rs3 << 2) | (rs4 << 3) | (DeviceSetting.DisableWLXTN << 4) | (DeviceSetting.DisableWLXTT << 5) | (DeviceSetting.DisableWCAM << 6) | (DeviceSetting.DisableWRFID << 7));// SystemSetting.data.SlienceMode;

	/* camera number 0~8 */
	buf.data[31] = GetCameraNumber();

	/* time send gps when stop */
	buf.data[32] = SystemSetting.data.FrqStop & 0xFF;
	buf.data[33] = (SystemSetting.data.FrqStop >> 8) & 0xFF;

	/* time capture image when move */
	buf.data[34] =  SystemSetting.data.FrqCaptureRun & 0xFF;
	buf.data[35] = (SystemSetting.data.FrqCaptureRun >> 8) & 0xFF;

	/* time capture image when stop/park */
	buf.data[36] =  SystemSetting.data.FrqCaptureStop & 0xFF;
	buf.data[37] = (SystemSetting.data.FrqCaptureStop >> 8) & 0xFF;

	/* pulse rate coefficient */
	//	buf.data[38] =  DeviceSetting.PulseKm & 0xFF;
	//	buf.data[39] = (DeviceSetting.PulseKm >> 8) & 0xFF;
	buf.data[38] =  SystemSetting.data.time_pw_off_camera_when_park & 0xFF;
	buf.data[39] = (SystemSetting.data.time_pw_off_camera_when_park >> 8) & 0xFF;

	/* baurate 2 bps */
	tmp32 = !SystemSetting.data.disable_pw_off_cam_when_park;
	buf.data[40] = tmp32 & 0xFF;
	buf.data[41] = (tmp32 >> 8) & 0xFF;
	buf.data[42] = (tmp32 >> 16) & 0xFF;
	buf.data[43] = (tmp32 >> 24) & 0xFF;

	/* baurate 2 bps */
//	buf.data[40] = DeviceSetting.BauRate2 & 0xFF;
//	buf.data[41] = (DeviceSetting.BauRate2 >> 8) & 0xFF;
//	buf.data[42] = (DeviceSetting.BauRate2 >> 16) & 0xFF;
//	buf.data[43] = (DeviceSetting.BauRate2 >> 24) & 0xFF;


	tmp32 = pw_camera_status;
	buf.data[44] = tmp32 & 0xFF;
	buf.data[45] = (tmp32 >> 8) & 0xFF;
	buf.data[46] = (tmp32 >> 16) & 0xFF;
	buf.data[47] = (tmp32 >> 24) & 0xFF;

	/* baurate 1 bps */
//	buf.data[44] = DeviceSetting.BauRate4 & 0xFF;
//	buf.data[45] = (DeviceSetting.BauRate4 >> 8) & 0xFF;
//	buf.data[46] = (DeviceSetting.BauRate4 >> 16) & 0xFF;
//	buf.data[47] = (DeviceSetting.BauRate4 >> 24) & 0xFF;

	/* Early warning time for driver continue */
	buf.data[48] =  SystemSetting.data.DrvForeWarningTime & 0xFF;
	buf.data[49] = (SystemSetting.data.DrvForeWarningTime >> 8) & 0xFF;

	/* time set 1 time stop/parking */
	buf.data[50] =  SystemSetting.data.DrvStopTime & 0xFF;
	buf.data[51] = (SystemSetting.data.DrvStopTime >> 8) & 0xFF;

	/* reserved */
	buf.data[52] = (SystemSetting.data.disable_audio) | (SystemSetting.data.disable_cam << 1) | (SystemSetting.data.dis_sys_rst_when_dis_cam << 2) | (SystemSetting.data.disable_capture << 3) | (SystemSetting.data.disable_camera_roaming_mode << 4);

	buf.data[53] = (SystemSetting.data.time_reset_when_disconnect >> 8) & 0xFF;
	buf.data[54] = (SystemSetting.data.time_reset_when_disconnect >> 8) & 0xFF;

	buf.data[55] = SystemSetting.data.filter_level;

	buf.data[56] = MobileCountryCode & 0xFF;
	buf.data[57] = (MobileCountryCode >> 8) & 0xFF;

	buf.data[58] = 0;
	buf.data[59] = 0;
	buf.data[60] = 0;
	buf.data[61] = 0;

	/* port - tcp server */
	buf.data[62] = SystemSetting.data.port & 0xFF;
	buf.data[63] = (SystemSetting.data.port >> 8) & 0xFF;
	tcp_len = 64;

	/* domain - tcp server */
	tmp = strlen(SystemSetting.data.domain);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.domain, tmp);
	tcp_len += tmp;

	/* device manufacture date */
	tmp = strlen(DeviceSetting.ManufactureDate);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, DeviceSetting.ManufactureDate, tmp);
	tcp_len += tmp;

	/* device update firmware - soft ware */
	tmp = strlen(DeviceSetting.IMEI);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, DeviceSetting.IMEI, tmp);
	tcp_len += tmp;

//	/* device update firmware - soft ware */
//	tmp = strlen(DeviceSetting.DeviceUpdatedDate);
//	buf.data[tcp_len] = tmp;
//	tcp_len++;
//	memcpy(buf.data + tcp_len, DeviceSetting.DeviceUpdatedDate, tmp);
//	tcp_len += tmp;

	/* password setting device */
	tmp = strlen(SystemSetting.data.PassWord);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.PassWord, tmp);
	tcp_len += tmp;

	/* number plate */
	tmp = strlen(DeviceSetting.NumberPlate);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, DeviceSetting.NumberPlate, tmp);
	tcp_len += tmp;

	/* URL download firmware pack */
	tmp = strlen(SystemSetting.data.update_fw_url);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.update_fw_url, tmp);
	tcp_len += tmp;

	/* URL upload image to server */
	tmp = strlen(SystemSetting.data.upload_file_url);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.upload_file_url, tmp);
	tcp_len += tmp;

	/* SIM IMSI current_operator */
	tmp = strlen(current_operator);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, current_operator, tmp);
	tcp_len += tmp;

#if (USE_ENCRYPT == 1)
	/* MODULE IMEI */
	tmp = strlen(DeviceSetting.IMEI);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, DeviceSetting.IMEI, tmp);
	tcp_len += tmp;
#else
	/* SIM ICCID current_mcc*/
	tmp = strlen(DeviceSetting.ICCID);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, DeviceSetting.ICCID, tmp);
	tcp_len += tmp;
#endif 	/* USE_ENCRYPT */

	/* phone number sms setting 1 */
	tmp = strlen(SystemSetting.data.Phone1);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.Phone1, tmp);
	tcp_len += tmp;

	/* phone number sms setting 2 */
	tmp = strlen(SystemSetting.data.Phone2);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.Phone2, tmp);
	tcp_len += tmp;

	/* phone number SOS */
	tmp = strlen(SystemSetting.data.Phone_sos);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.Phone_sos, tmp);
	tcp_len += tmp;

	/* motion value */
	sprintf(str_motion_value, "%.2f\0", SystemSetting.data.MotionValue);
	tmp = strlen(str_motion_value);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, str_motion_value, tmp);
	tcp_len += tmp;

	/* camera serial number list */
	memset(cam_sn_list, 0x0, 128);
	get_list_camera_sn(cam_sn_list);

	tmp = strlen(cam_sn_list);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, cam_sn_list, tmp);
	tcp_len += tmp;

	/* Key Active */
	tmp = strlen(SystemSetting.data.KeyActive);
	buf.data[tcp_len] = tmp;
	tcp_len++;
	memcpy(buf.data + tcp_len, SystemSetting.data.KeyActive, tmp);
	tcp_len += tmp;

	/* Length data */
	tcp_len += 1;//crc len
	buf.data[11] = tcp_len & 0xFF;
	buf.data[12] = tcp_len >> 8;

	/* CRC */
	crc = CRC_8BitsCompute(buf.data, tcp_len - 1);
	buf.data[tcp_len - 1] = crc;

	buf.size = tcp_len;
	buf.Data_Type = DEVICE_SETTING;
	buf.trace = 0;

	Push_TCP_Data(buf);
}

static uint8_t checkPositionArea(uint8_t index, float latGPS, float longGPS){
	if(index >= TOTOL_AREA) return 0;

	int i, j, c = 0;
	for (i = 0, j = bufNumberArea[index][1] - 1; i < bufNumberArea[index][1]; j = i++) {
		if(((bufNumberArea[index][2+2*i] > longGPS) != (bufNumberArea[index][2+2*j] > longGPS)) && (latGPS < (bufNumberArea[index][3+2*j] - bufNumberArea[index][3+2*i]) * (longGPS - bufNumberArea[index][2+2*i]) / (bufNumberArea[index][2+2*j] - bufNumberArea[index][2+2*i]) + bufNumberArea[index][3+2*i]))
			c = !c;
	}
	return c;
}

static void init_gps_data(void){
	time_t rawtime;
	time( &rawtime );
	GPS_Data.fact_time = 0;
	GPS_Data.gps_speed = 0;
	GPS_Data.pulse_speed = 0;
	GPS_Data.gps_status = 0;
	GPS_Data.timestamp = rawtime + GMT_HN;
	GPS_Data.gps_time = GPS_Data.sys_time = localtime( &rawtime );
	GPS_Data.location.latitude = 0.0;
	GPS_Data.location.longitude = 0.0;
	GPS_Data.gps_status = 0;
	GPS_Data.prv_location.latitude = 0;
	GPS_Data.prv_location.longitude = 0;
}

uint8_t speed_limit_device = 0;
uint8_t isFlagOnHighWay = 0;
uint8_t index_Area = 0;
static u8 gps_idx = 0;
static double distance_tmp = 0;
static double distance_lv = 0;
static u8 time_get_location = 0;
static u16 cnt_rst_park = 0;
u8 sp_pecent = 0;
struct timeval tv_n;
u8 flg_set_sys_time = 0;

double param_lat = 0;

static void ql_loc_rx_ind_msg_cb(loc_client_handle_type  h_loc,
                                 E_QL_LOC_NFY_MSG_ID_T   e_msg_id,
                                 void                    *pv_data,
                                 void                    *contextPtr){
    switch(e_msg_id){
		case E_QL_LOC_NFY_MSG_ID_STATUS_INFO:{
			char *status[]  = {"NONE",  "SESSION_BEGIN",  "SESSION_END",  "ENGINE_ON",  "ENGINE_OFF"};
			E_QL_LOC_STATUS_VALUE_T e_status = *((E_QL_LOC_STATUS_VALUE_T*)pv_data);
			log_system("GPS status=%s\n", status[e_status]);
			break;
		}
        case E_QL_LOC_NFY_MSG_ID_LOCATION_INFO:{
        	time_t rawtime;
        	time_t gpstime;

            QL_LOC_LOCATION_INFO_T *pt_location = (QL_LOC_LOCATION_INFO_T *)pv_data;

			GPS_Data.gps_status = 1;

            GPS_Data.gps_time_out = 0;

            GPS_Data.tmp_speed = pt_location->speed * (float)3.6;

            GPS_Data.timestamp = ((pt_location->timestamp / 1000) + GMT_HN);// - GPS_Data.fact_time;

            gpstime = (pt_location->timestamp / 1000);// - GPS_Data.fact_time;

            if((flg_set_sys_time == 0) || ((GPS_Data.timestamp % 3600) == 0)){
                tv_n.tv_sec = (pt_location->timestamp / 1000);
                tv_n.tv_usec = 0;

                if (!settimeofday(&tv_n, NULL)) {
                	flg_set_sys_time = 1;
                	log_system("###SYSTEM TIME SETTING SUCCESS###\n");
                }else{
                	log_system("###SYSTEM TIME SETTING ERROR###\n");
                }
            }

			time( &rawtime );

			GPS_Data.sys_time = localtime( &rawtime );

			GPS_Data.gps_time = localtime( &gpstime );

			//log_system("WDAY %d",  GPS_Data.gps_time->tm_wday);

			camera_count_ ++;

            if((ACC_STATUS == 1) || (GPS_Data.location.latitude == 0) || (GPS_Data.location.longitude == 0)){
            	if(ACC_STATUS == 0){
            		if(time_get_location < 30){
						time_get_location++;
						break;
            		}
            	}

            	GPS_Data.location.latitude = pt_location->latitude;
            	GPS_Data.location.longitude = pt_location->longitude;

//            	if(speed_test != 0){
//            		GPS_Data.location.latitude += param_lat;
//            		param_lat += (float)0.00005;
//            	}

				GPS_Data.gps_speed = pt_location->speed * 3.6;

				/* check flg decrease speed */
				if(DeviceSetting.Decrease_Speed == 1){
					sp_pecent = (GPS_Data.gps_speed * 10)/100;
					if(GPS_Data.gps_speed > 55){
						GPS_Data.gps_speed = GPS_Data.gps_speed - sp_pecent;
					}
				}

				GPS_Data.bearing = pt_location->bearing;
				distance_tmp = haversine(GPS_Data.location.latitude, GPS_Data.location.longitude, GPS_Data.prv_location.latitude, GPS_Data.prv_location.longitude);

				distance_lv = ((float)SystemSetting.data.RunLevel * (float)0.27778);

				if((distance_tmp >= distance_lv) && (distance_tmp < 50.0)){

					/* increment total km in day */
					tibase.GPSMeterInday += distance_tmp;
				}

				GPS_Data.prv_location = GPS_Data.location;
            }else{
            	GPS_Data.gps_speed = 0;

            	if(DeviceSetting.device_reset_when_park != 0){
            		cnt_rst_park ++;
            		if(cnt_rst_park >= 300){
            			cnt_rst_park = 0;

            			DeviceSetting.device_reset_when_park --;
            			write_number(DEV_RST_WHEN_PARK_PATH, (u64)DeviceSetting.device_reset_when_park);

    					reboot_camera();
    					/* reboot device */
    					set_device_reset_timeout(30);
            		}
            	}
            }

			//if(speed_test != 0) GPS_Data.gps_speed = speed_test;

			if(GPS_Data.gps_speed > DeviceSetting.SpeedLimit){
				while(1){
					if(checkPositionArea(index_Area, GPS_Data.location.latitude, GPS_Data.location.longitude) == 1){
						isFlagOnHighWay = 1;
						break;
					}
					
					if(isFlagOnHighWay == 1){
						isFlagOnHighWay = 0;
						index_Area = 0;
						continue;
					}else{
						index_Area ++;
						if(index_Area == TOTOL_AREA){
							index_Area = 0;
							break;
						}
					}
				}
				if(isFlagOnHighWay == 1){
					speed_limit_device = 150;
				}else{
					speed_limit_device = DeviceSetting.SpeedLimit;
				}
			}else{
				speed_limit_device = DeviceSetting.SpeedLimit;
			}

            gps_idx = (GPS_Data.timestamp % SystemSetting.data.FrqRun);
			if(gps_idx == 0){
				GPS_Data.speeds[SystemSetting.data.FrqRun - 1] = GPS_Data.gps_speed;
				GPS_Data.locations[SystemSetting.data.FrqRun - 1] = GPS_Data.location;
			}else{
				GPS_Data.speeds[gps_idx - 1] = GPS_Data.gps_speed;
				GPS_Data.locations[gps_idx - 1] = GPS_Data.location;
			}

            /* send real time data (gps signal = 1) */
            if((GPS_Data.freq_send > 0) && ((GPS_Data.timestamp % GPS_Data.freq_send) == 0)){
				/* wite gps data to database */
				write_gps_data();

				/* send gps data to server */
				if((network_data.tcp_status) && (SystemSetting.data.DeviceID != 0)){
					send_realtime_data();
				}

				memset(GPS_Data.speeds, 0x0, GPS_PER_SECOND);
				memset(&GPS_Data.locations, 0x0, sizeof(GPS_Data.locations));
				GPS_Data.flg_update_cam = true;
            }

			/* check over speed */
			if(GPS_Data.gps_speed > speed_limit_device){
				if(GPS_Data.ovSpeedData.ovSpeedHold == 0){
					GPS_Data.ovSpeedData.StartTime = GPS_Data.timestamp;
					GPS_Data.ovSpeedData.StartLocation = GPS_Data.location;
				}

				GPS_Data.ovSpeedData.ovSpeedHold += 1;

				GPS_Data.ovSpeedData.speeds += GPS_Data.gps_speed;
				if((GPS_Data.ovSpeedData.ovSpeedHold == 3) || (GPS_Data.ovSpeedData.ovSpeedHold % 15) == 0){
					add_play_list("/usrdata/sound/", "overspeed.wav", SystemSetting.data.SpeakerVolume);
				}
				tibase.over_speed_status = true;
			}else{
				tibase.over_speed_status = false;

				/* Over speed 20 second */
				if(GPS_Data.ovSpeedData.ovSpeedHold >= 29){
					SendOverSpeed();
				}
				GPS_Data.ovSpeedData.ovSpeedHold = 0;
			}

			/* clear New Week on new Version */
			if(DeviceSetting.flg_clear_data_new_ver != 2){
				tibase.DriverInWeek = 0;
				if(update_tibase(tibase) == 0){
					DeviceSetting.flg_clear_data_new_ver = 2;
					write_number(FLG_CLEAR_NEW_VER_PATH, (u64)DeviceSetting.flg_clear_data_new_ver);
					//log_system("XOA THANH CONG DU LIEU TRONG TUAN KHI NANG CAP VER MOI \n");
				}

				clear_summary_data(1, 1);
			}

			//log_system("THIS WEEK => %d, FACT %llu \n", (u8)get_week_of_year(GPS_Data.gps_time), GPS_Data.fact_time);

			if(DeviceSetting.CurrentDay != GPS_Data.gps_time->tm_mday){
				if(((GPS_Data.gps_time->tm_year != 80) && (GPS_Data.gps_time->tm_mday > 0) && (GPS_Data.gps_time->tm_mday <= 31))){
					int week_cal = get_week_of_year(GPS_Data.gps_time);
					if(DeviceSetting.CurrentWeek != week_cal){
						log_system("GO TO NEW WEEK %d - %d!!!!!\n", DeviceSetting.CurrentWeek, week_cal);

						/* set flg new week in drive_cal */
						GPS_Data.flg_new_week = 1;
					}

					log_system("GO TO NEW DAY %d - %d!!!!!\n", DeviceSetting.CurrentDay, GPS_Data.gps_time->tm_mday);

					if((GPS_Data.gps_time->tm_min >= 15) || (GPS_Data.gps_time->tm_hour != 0)){
						clear_driver_result(0, 0);
					}

					/* set flg new day in drive_cal */
					GPS_Data.flg_new_day = 1;

					count_warning_cam_inday = 0;
					write_number(COUNT_WARNING_CAM_PATH, (u64)count_warning_cam_inday);

					/* reboot camera */
					reboot_camera();

					/* reset count of dev rst */
					clear_time_reset_increment();

					/* reboot device */
					set_device_reset_timeout(120);
				}
			}

            /* driver data calculate */
            calculate_drive_data();

			break;
        }
        case E_QL_LOC_NFY_MSG_ID_SV_INFO:
        {
            int i, svs_in_use = 0;
            QL_LOC_SV_STATUS_T  *pt_svinfo = (QL_LOC_SV_STATUS_T  *)pv_data;

            for(i=0; i<pt_svinfo->num_svs; i++)
            {
            	if((pt_svinfo->used_in_fix_mask&(1<<i)))
            		svs_in_use++;
            }
            GPS_Data.svs_in_use = svs_in_use;
            GPS_Data.svs_in_view = pt_svinfo->num_svs;
            log_debug("SVS in view: %d, SVS in use: %d\n", GPS_Data.svs_in_view,  GPS_Data.svs_in_use);
            break;
        }
        case E_QL_LOC_NFY_MSG_ID_NMEA_INFO:
        	 break;
        case E_QL_LOC_NFY_MSG_ID_CAPABILITIES_INFO:
            break;
        case E_QL_LOC_NFY_MSG_ID_AGPS_STATUS:
            break;
        case E_QL_LOC_NFY_MSG_ID_NI_NOTIFICATION:
            break;
        case E_QL_LOC_NFY_MSG_ID_XTRA_REPORT_SERVER:
            break;
        default:
        	log_gps("GPS DATA NOT IN CASE\n");
        	break;
    }
}

void get_mcu_devsetting(void){
	u8 tmp8 = 0;
	u16 tmp16 = sizeof(sys_setting);
	u8 mcu_data[16];
	mcu_data[0] = '>';   //0x1
	mcu_data[1] = '>';
	mcu_data[2] = 0x05;

	log_system("GET DEVICE SETTING FROM MCU MEMORY \n");

	mcu_data[3] = (tmp16 >> 8) & 0xFF;
	mcu_data[4] =  tmp16 & 0xFF;

	//CRC
	tmp8 = CRC_8BitsCompute(mcu_data, 5);
	mcu_data[5] = tmp8;
	mcu_send_msg(mcu_data, 6);
}

u8 mcu_cfg_data[512];
void update_mcu_devsetting(void){
	u8 tmp8 = 0;
	u16 msg_len = 0;
	u16 tmp16 = sizeof(sys_setting);
	u32 crc32 = 0;

	if(SystemSetting.data.DeviceID == 0) return;

	mcu_cfg_data[0] = '>';   //0x1
	mcu_cfg_data[1] = '>';
	mcu_cfg_data[2] = 0x04;

	mcu_cfg_data[3] = ((tmp16 + 4) >> 8) & 0xFF;
	mcu_cfg_data[4] =  (tmp16 + 4) & 0xFF;

	msg_len = 5;

	memcpy(mcu_cfg_data + msg_len, SystemSetting.bytes, tmp16);
	msg_len = msg_len + tmp16;

	crc32 = CRC32Software(SystemSetting.bytes, (long)tmp16);

	mcu_cfg_data[msg_len] = (crc32 >> 24) & 0xFF;
	msg_len = msg_len + 1;
	mcu_cfg_data[msg_len] = (crc32 >> 16) & 0xFF;
	msg_len = msg_len + 1;
	mcu_cfg_data[msg_len] = (crc32 >> 8) & 0xFF;
	msg_len = msg_len + 1;
	mcu_cfg_data[msg_len] = crc32 & 0xFF;
	msg_len = msg_len + 1;

	//CRC
	tmp8 = CRC_8BitsCompute(mcu_cfg_data, msg_len);
	mcu_cfg_data[msg_len] = tmp8;
	msg_len = msg_len + 1;

	mcu_send_msg(mcu_cfg_data, msg_len);

	log_system("STORE DEVICE SETTING TO MCU MEMORY CRC = %u, Len = %d, data = %d \n", crc32, msg_len, tmp16);
}

typedef enum{
    STATE_LOC_INIT = 0,
    STATE_ADD_RX_HANDLER = 1,
    STATE_SET_INDICATION = 2,
	STATE_SET_POSITION_MODE = 3,
    STATE_START_NAVIGATION = 4,
	STATE_GPS_READY = 5,
	STATE_GPS_ERROR = 6
}Enum_GPSSTATE;

Enum_GPSSTATE gps_init_state = STATE_LOC_INIT;
u8 transfer_setting_mcu = 0;
void* thread_gps(void* arg){
	int ret         = E_QL_OK;
	int h_loc       = 0;
	//int time_to_update_data = 0;
	QL_LOC_POS_MODE_INFO_T  t_mode      = {0};
	QL_LOC_LOCATION_INFO_T  t_loc_info  = {0};
    //setting time zone to GMT +7
	set_tz_gmt_hn();
	memset(&GPS_Data, 0x0, sizeof(GPS_typedef));

	memset(&MCU_Data, 0x0, sizeof(mcu_type_def));

	/* set default frequency send */
	GPS_Data.freq_send = FREQ_WRITE_GPS_RUN;

	/* send device config to server */
	GPS_Data.flg_send_cfg = 1;
	MCU_Data.last_Hard_Acc = 2;

	/* set default gps data */
	init_gps_data();

	/* set camera status default = true */
	network_data.cam_status = true;

	for(;;){
		/* calculator frequency send gps data  */
		cal_tcp_freq();
		/* init gps service */
		if(STATE_GPS_READY != gps_init_state){
			switch(gps_init_state){
				case STATE_LOC_INIT:{
					ret = QL_LOC_Client_Init(&h_loc);
					log_gps("QL_LOC_Client_Init ret %d with h_loc=%d\n", ret, h_loc);
					if(0 != ret){
						log_gps("QL_LOC_Client_Init FAILED\n");

						gps_init_state = STATE_GPS_ERROR;
						break;
					}
					gps_init_state = STATE_ADD_RX_HANDLER;
					break;
				}
				case STATE_ADD_RX_HANDLER:{
					ret = QL_LOC_AddRxIndMsgHandler(ql_loc_rx_ind_msg_cb, (void*)h_loc);
					log_gps("QL_LOC_AddRxIndMsgHandler ret %d\n", ret);
					if(0 != ret){
						log_gps("QL_LOC_AddRxIndMsgHandler FAILED\n");
						gps_init_state = STATE_GPS_ERROR;
						break;
					}
					gps_init_state = STATE_SET_INDICATION;
					break;
				}
				case STATE_SET_INDICATION:{
					/* Set what we want callbacks for */
					ret = QL_LOC_Set_Indications(h_loc, LOCATON_BITMASK);
					log_gps("QL_LOC_Set_Indications ret %d\n", ret);
					if(0 != ret){
						log_gps("QL_LOC_Set_Indications FAILED\n");
						gps_init_state = STATE_GPS_ERROR;
						break;
					}
					gps_init_state = STATE_SET_POSITION_MODE;
					break;
				}
				case STATE_SET_POSITION_MODE:{
					t_mode.mode                 = E_QL_LOC_POS_MODE_STANDALONE;
					t_mode.recurrence           = E_QL_LOC_POS_RECURRENCE_PERIODIC;
					t_mode.min_interval         = 1000;  //report nmea frequency 1Hz
					t_mode.preferred_accuracy   = 50;    // <50m
					t_mode.preferred_time       = 90;    // 90s
					ret = QL_LOC_Set_Position_Mode(h_loc, &t_mode);
					log_gps("QL_LOC_Set_Position_Mode ret %d\n", ret);

					if(0 != ret){
						log_gps("QL_LOC_Set_Position_Mode FAILED\n");

						gps_init_state = STATE_GPS_ERROR;
						break;
					}
					gps_init_state = STATE_START_NAVIGATION;
					break;
				}
				case STATE_START_NAVIGATION:{
					ret = QL_LOC_Start_Navigation(h_loc);
					log_gps("QL_LOC_Start_Navigation ret=%d\n", ret);

					if(0 != ret){
						log_gps("QL_LOC_Start_Navigation FAILED\n");
						gps_init_state = STATE_GPS_ERROR;
						break;
					}
					if(!access("/usrdata/xtra2.bin", F_OK)){
			            ret = QL_LOC_Xtra_InjectFile(h_loc, "/usrdata/xtra2.bin");
			            log_system("QL_LOC_Xtra_InjectFile ret=%d\n", ret);

			            QL_LOC_AGPS_DATA_CONN_OPEN_INTO_T t_info = {0};
			            //You should fill t_info with right value
			            ret = QL_LOC_Agps_DataConnOpen(h_loc, &t_info);
			            log_system("QL_LOC_Agps_DataConnOpen ret=%d\n", ret);
					}

					gps_init_state = STATE_GPS_READY;
				}
				case STATE_GPS_READY:{
					time_t rawtime;
					time( &rawtime );
					GPS_Data.gps_time = GPS_Data.sys_time = localtime( &rawtime );
					GPS_READY = 1;
					break;
				}
				case STATE_GPS_ERROR:{
					log_system("################# START GPS FAILED -> REBOOT ###############\n");
					set_device_reset_timeout(10);
					break;
				}
			}
		}else{
			GPS_Data.tm_up++;
			GPS_Data.gps_time_out++;

			if(GPS_Data.no_acc_time <= 3700){
				GPS_Data.no_acc_time++;
			}

			if((GPS_Data.tm_up > 0) && (GPS_Data.tm_up % 5) == 0){
				if(SystemSetting.data.DeviceID == 0){
					/* get data when device id = 0 */
					get_mcu_devsetting();
				}else if(MCU_Data.MCU_Verison != 0){
					if(transfer_setting_mcu == 0){
						transfer_setting_mcu = 1;
						update_mcu_devsetting();
					}
				}

				update_mcu_gps_data();
				update_rfid_status();

				if((GPS_Data.tm_up % TIME_10_MINUTES) == 0){
					send_device_setting();
				}
			}

			if(GPS_Data.cam_up_time <= 600){
				GPS_Data.cam_up_time++;
				if((GPS_Data.cam_up_time % 20) == 0){
					GPS_Data.flg_scan_cam = true;
				}
			}

			if((GPS_Data.tm_up > 0) && ((GPS_Data.tm_up % 3600) == 0)){
				GPS_Data.flg_reboot_sgmi = true;
			}

            if((GPS_Data.freq_send_camera > 0) && (camera_count_ >= GPS_Data.freq_send_camera) && (SystemSetting.data.disable_capture == 0)){
            	camera_count_ = 0;
            	GPS_Data.flg_capture = true;
            }

			if(GPS_Data.gps_time_out > 10){/* gps message timeout - loss signal */
				time_t rawtime;
				time( &rawtime );
				camera_count_ ++;
				GPS_Data.gps_speed = 0;
				GPS_Data.tmp_speed = 0;
				GPS_Data.gps_status = 0;
				GPS_Data.timestamp = rawtime + GMT_HN;
				GPS_Data.gps_time = GPS_Data.sys_time = localtime( &rawtime );
				memset(GPS_Data.speeds, 0x0, GPS_PER_SECOND);

				if((GPS_Data.location.latitude == 0) || (GPS_Data.location.longitude == 0)){
					if((tibase.b_loc_stop.latitude == 0) && (tibase.b_loc_stop.longitude == 0)){
						GPS_Data.location = tibase.b_loc_stop;
					}
				}

				/* send real time data (gps signal = 0) */
				if((GPS_Data.freq_send > 0) && ((rawtime % GPS_Data.freq_send) == 0)){
					char mcu_msg[128];

	            	/* send gps data to server */
	            	if(network_data.tcp_status){
	            		send_realtime_data();
	            	}
					sprintf(mcu_msg, "LOSS GPS - SYSTEM TIME : %s\n", asctime(GPS_Data.sys_time));
					log_gps(mcu_msg);
				}

				/* wite gps data to file */
				if((rawtime % FREQ_WRITE_LOSS_GPS) == 0){
					write_gps_data();
				}

				tibase.over_speed_status = false;
				tibase.over_driver_continue_status = false;

				if((GPS_Data.gps_time_out > TIME_15_MINUTES) && (get_run_time() > TIME_15_MINUTES)){
					log_system("##### clear driver result when device loss GPS than more 15 Minute #####\n");
					/* Write driver data */
					write_driver_data(STATUS_RUNNING, (time_t)tibase.bRunTime, (time_t)tibase.bStopTime, GPS_Data.location, (u8)GPS_Data.gps_time->tm_mday);

					/* Reset time driving pre time */
					tibase.RunSecondsPrvDay = 0;

					clear_driver_result(0, 0);
				}

				if(GPS_Data.gps_time_out >= get_frq_reset()){
					/* increase time reset */
					increase_frq_reset();

					GPS_Data.gps_time_out = 0;

					//reboot device
					log_system("################# LOSS GPS -> REBOOT ###############\n");

					set_device_reset_timeout(10);
				}
			}
		}

		MCU_Data.pwr_warning_delay_tm++;
		if(MCU_Data.pwr_warning_delay_tm > 200){
			MCU_Data.pwr_warning_delay_tm = 100;
		}

		if(GPS_Data.flg_send_cfg == 1){
			GPS_Data.time_to_send_evt ++;
			if(GPS_Data.time_to_send_evt >= 120){
				GPS_Data.time_to_send_evt = 0;
				GPS_Data.flg_send_cfg = 0;
				/* send device setting to server */
				send_device_setting();
			}
		}

		sleep(1);
	}
}

void clear_camera_cnt(void){
	camera_count_ = 0;
}

u8 gps_ready(void){
	return (gps_init_state == STATE_GPS_READY) ? 1 : 0;
}

