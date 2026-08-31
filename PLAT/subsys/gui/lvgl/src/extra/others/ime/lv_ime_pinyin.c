/**
 * @file lv_ime_pinyin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ime_pinyin.h"
#if LV_USE_IME_PINYIN != 0

#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS    &lv_ime_pinyin_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_ime_pinyin_style_change_event(lv_event_t * e);
static void lv_ime_pinyin_kb_event(lv_event_t * e);
static void lv_ime_pinyin_cand_panel_event(lv_event_t * e);

static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict);
static void pinyin_input_proc(lv_obj_t * obj);
static void pinyin_page_proc(lv_obj_t * obj, uint16_t btn);
static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num);
static void pinyin_ime_clear_data(lv_obj_t * obj);

#if LV_IME_PINYIN_USE_K9_MODE
    static void pinyin_k9_init_data(lv_obj_t * obj);
    static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[]);
    static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str);
    static void pinyin_k9_fill_cand(lv_obj_t * obj);
    static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_ime_pinyin_class = {
    .constructor_cb = lv_ime_pinyin_constructor,
    .destructor_cb  = lv_ime_pinyin_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .group_def      = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size  = sizeof(lv_ime_pinyin_t),
    .base_class     = &lv_obj_class
};

#if LV_IME_PINYIN_USE_K9_MODE
static char * lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21] = {\
                                                                                ",\0", "1#\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
                                                                                ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_KEYBOARD"\0", "\n\0",
                                                                                "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
                                                                                LV_SYMBOL_LEFT"\0", "\0"
                                                                               };

static lv_btnmatrix_ctrl_t default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 17] = { 1 };
static char   lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 2][LV_IME_PINYIN_K9_MAX_INPUT] = {0};
#endif

static char   lv_pinyin_cand_str[LV_IME_PINYIN_CAND_TEXT_NUM][4];
static char * lv_btnm_def_pinyin_sel_map[LV_IME_PINYIN_CAND_TEXT_NUM + 3];

#if LV_IME_PINYIN_USE_DEFAULT_DICT
lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    {"a","啊阿呵吖嗄腌锕"},
    {"ai","哎唉哀爱埃癌蔼矮碍挨艾皑隘捱嗳嗌嫒瑷暧砹锿霭"},
    {"an","安俺按暗岸案鞍氨胺谙埯揞犴庵桉铵鹌黯"},
    {"ang","昂盎肮"},
    {"ao","凹敖熬傲奥懊翱袄澳坳拗嗷岙廒遨媪骜獒聱螯鏊鳌鏖"},
    {"ba","爸八巴吧把拔坝罢霸叭扒芭疤捌笆跋靶茇菝岜灞钯粑鲅魃"},
    {"bai","白百摆败拜佰伯柏稗掰呗"},
    {"ban","办半班般板版搬伴颁斑扳扮拌绊瓣阪舨瘢钣"},
    {"bang","帮棒绑榜膀傍谤浜邦蚌磅镑梆"},
    {"bao","包报抱宝饱保暴爆剥薄雹堡豹曝煲雹刨褓鸨苞胞褒鲍"},
    {"bei","贝被北倍杯悲辈备背碑孛狈卑钡惫焙邶蓓悖呗碚鹎陂"},
    {"ben","奔本笨苯贲夯贲锛"},
    {"beng","崩绷甭泵迸蹦蚌嘣甏"},
    {"bi","比笔闭逼鼻必毕彼鄙弊碧蔽壁避臂币庇毖陛毙敝痹蓖辟陛匕俾荸狴弼愎璧裨贲吡铋秕婢薜"},
    {"bian","边便变遍编辨辩辫鞭贬扁卞匾蝙汴煸忭砭缏褊弁"},
    {"biao","表标彪飙飚膘婊飑镖杓镳瘭裱鳔骠"},
    {"bie","别憋鳖瘪蹩"},
    {"bin","宾彬斌滨濒缤傧殡膑镔髌鬓"},
    {"bing","冰兵柄并病饼炳丙秉禀摒邴摒槟"},
    {"bo","伯泊驳卜柏剥薄玻菠播拨钵波博勃搏铂箔帛舶脖膊渤孛亳蕃啵饽檗擘礴钹鹁簸趵跛踣"},
    {"bu","不补部布步捕簿吥卜埠怖哺埔卟逋瓿晡钚钸醭"},
    {"ca","擦嚓礤"},
    {"cai","才菜材财裁采猜彩睬踩蔡"},
    {"can","参惨餐惭残蚕灿掺璨粲骖黪孱"},
    {"cang","仓沧苍舱藏伧"},
    {"cao","草操糙曹槽嘈漕"},
    {"ce","册侧厕测策恻"},
    {"cen","参岑涔"},
    {"ceng","层蹭曾噌"},
    {"cha","查差插茶叉察诧刹茬岔搽嚓碴槎檫锸镲衩"},
    {"chai","柴拆差豺钗虿瘥侪龇"},
    {"chan","产缠掺搀阐颤铲谗蝉单馋觇婵蒇谄冁廛孱蟾羼镡忏潺禅骣躔澶丳亶佔僝"},
    {"chang","长唱常场厂尝肠畅昌敞倡偿猖裳鲳氅菖惝嫦徜鬯阊怅伥昶苌娼嫦昶裳"},
    {"chao","朝抄超吵潮巢炒嘲剿绰钞晁焯怊耖"},
    {"che","车扯撤掣彻澈坼屮砗"},
    {"chen","尘臣沉称辰陈晨衬趁橙琛宸嗔谌碜伧龀忱郴"},
    {"cheng","称撑成呈承诚城乘惩程澄橙逞骋秤丞瞠噌铛铖铛裎蛏酲"},
    {"chi","吃迟持尺池齿耻斥赤痴弛驰侈炽翅叱嗤蚩坻踟饬媸敕眵鸱瘛褫蚩螭笞篪豉踟魑"},
    {"chong","充冲虫崇宠憧忡重茺舂铳艟"},
    {"chou","抽仇愁丑臭筹酬畴瞅惆绸畴稠踌瞅帱雠"},
    {"chu","出初除处触厨楚畜滁锄雏橱躇础储搐矗黜杵蜍楮"},
    {"chuai","踹揣搋嘬膪"},
    {"chuan","川穿传船串喘揣踹椽巛氚钏舡"},
    {"chuang","闯疮窗床创怆"},
    {"chui","吹炊垂捶锤陲棰槌"},
    {"chun","春纯唇蠢鹑淳醇椿"},
    {"chuo","戳绰辍啜龊踔"},
    {"ci","次此刺词辞赐磁瓷雌祠慈伺疵茨偨"},
    {"cong","从丛聪葱囱匆苁淙骢琮璁枞"},
    {"cou","凑楱辏腠"},
    {"cu","粗醋簇猝促蔟徂殂酢蹙蹴"},
    {"cuan","蹿窜篡攒汆撺爨镩"},
    {"cui","脆崔催摧淬瘁粹翠萃啐悴璀榱毳隹"},
    {"cun","村存寸蹲忖皴"},
    {"cuo","错措搓磋撮挫鹾痤蹉锉矬瘥鹾蹉躜"},
    {"da","大打搭达答耷哒嗒怛妲沓疸褡笪靼鞑"},
    {"dai","呆歹傣代带待怠殆贷袋逮戴岱迨骀绐玳黛"},
    {"dan","但丹单担耽郸胆掸旦诞弹惮淡蛋氮赡石儋萏啖澹殚赕眈疸瘅聃箪"},
    {"dang","当挡党荡档宕砀铛裆"},
    {"dao","刀导岛倒捣祷蹈到悼盗道稻焘纛"},
    {"de","的得地德锝"},
    {"dei","得"},
    {"deng","灯登等瞪凳邓澄蹬噔嶝戥磴镫簦"},
    {"di","地弟帝低堤滴狄迪敌涤笛嫡底抵递第缔蒂提氐籴诋谛邸坻荻嘀娣柢棣觌砥碲睇镝羝骶"},
    {"dia","嗲"},
    {"dian","电典点掂店垫惦淀奠殿滇颠碘佃甸靛阽坫巅玷钿癜癫簟踮"},
    {"diao","刁掉吊钓貂调碉叼雕凋铞铫鲷"},
    {"die","爹碟蝶跌迭谍叠垤堞揲喋牒瓞耋蹀鲽"},
    {"ding","丁定订叮钉顶鼎锭仃啶盯玎腚碇町铤疔耵酊"},
    {"diou","丢铥"},
    {"dong","东冬董懂冻洞咚动栋侗恫岽峒氡胨胴硐鸫"},
    {"dou","斗都陡豆逗痘兜抖蔸窦蚪篼"},
    {"du","都督毒犊肚度独读堵睹赌杜镀渡妒芏嘟渎椟牍蠹笃髑黩"},
    {"duan","段短断端锻缎簖"},
    {"dui","堆队对兑怼憝碓"},
    {"dun","吨敦墩蹲盾钝顿遁炖礅盹镦趸"},
    {"duo","多哆夺掇朵垛躲剁堕舵惰跺哚沲缍铎裰踱"},
    {"e","阿蛾峨鹅俄额讹娥恶厄扼遏鄂饿哦噩谔垩苊莪萼呃愕屙婀轭腭锇锷鹗颚鳄"},
    {"ei","诶"},
    {"en","恩嗯蒽摁"},
    {"er","二而儿耳尔饵洱贰迩珥铒鸸鲕"},
    {"fa","发法罚筏伐乏阀珐垡砝"},
    {"fan","反返范繁贩犯饭泛凡烦藩帆番翻樊矾钒蕃蘩幡梵燔畈蹯"},
    {"fang","方放房芳纺坊防妨肪仿访邡枋钫舫鲂"},
    {"fei","飞非废沸肺费肥匪啡菲诽吠妃绯榧贲腓斐扉砩镄痱蜚篚翡霏鲱"},
    {"fen","分纷奋份坟焚芬粉忿愤粪酚吩氛汾偾瀵玢棼贲鲼鼢"},
    {"feng","丰风枫封疯峰凤烽锋蜂冯逢缝讽奉俸酆葑唪沣砜"},
    {"fo","佛"},
    {"fou","否缶"},
    {"fu","父夫服符伏福负富府敷肤腐赴副覆赋复傅附妇缚浮孵扶孚抚辅俯釜斧驸拂辐幅氟俘涪袱弗甫脯腑付阜腹讣咐匐凫阝郛芙芾苻茯莩菔拊呋幞怫滏艴绂绋桴赙祓砩黻黼罘稃馥蚨蜉蝠蝮麸趺跗鲋鳆"},
    {"ga","噶嘎夹咖伽尬尕尜旮钆"},
    {"gai","该改概钙盖溉芥丐陔垓戤赅胲"},
    {"gan","干赶感秆敢竿肝甘杆柑赣坩苷尴擀泔淦澉绀橄旰矸疳酐"},
    {"gang","刚钢缸肛纲岗冈港杠扛戆罡筻"},
    {"gao","高膏告羔糕搞镐稿篙皋睾诰郜藁缟槔槁杲锆"},
    {"ge","个各哥歌搁戈鸽胳疙割革葛格蛤阁隔铬合咯鬲仡哿圪塥嗝纥搿膈铪镉袼虼舸骼"},
    {"gei","给"},
    {"gen","根跟亘茛哏艮"},
    {"geng","更耕庚羹埂耿梗颈哽赓绠鲠"},
    {"gong","工攻公宫贡共弓功恭龚供躬巩汞拱廾珙肱蚣觥"},
    {"gou","够钩勾沟苟狗垢构购佝诟岣遘媾缑枸觏彀笱篝鞲"},
    {"gu","古谷股骨蛊鼓固故顾雇贾嘏诂菰崮汩梏轱牯牿臌毂瞽罟钴锢鸪鹄痼蛄酤觚鲴鹘"},
    {"gua","瓜刮剐寡挂褂"},
    {"guai","怪乖拐掴"},
    {"guan","关官冠观管馆惯灌贯罐棺纶倌莞掼涫盥鹳鳏"},
    {"guang","光广逛咣犷桄胱"},
    {"gui","归龟闺轨鬼瑰跪贵刽规圭硅诡癸桂柜炔匦刿庋宄妫桧炅晷皈簋鲑鳜"},
    {"gun","滚棍辊衮绲磙鲧"},
    {"guo","国果裹过锅郭涡馘埚掴呙帼崞猓椁虢锞聒蜾蝈"},
    {"ha","哈铪蛤"},
    {"hai","嗨孩还海害氦骸亥骇咳胲醢"},
    {"han","汉韩含捍旱汗涵寒函喊罕翰酣憨邯撼憾悍焊邗菡撖瀚晗焓顸颔蚶鼾"},
    {"hang","夯杭航吭巷行沆绗颃"},
    {"hao","好号浩昊豪毫郝耗镐壕嚎貉蒿薅嗥嚆濠灏皓颢蚝"},
    {"he","呵喝和何合盒荷菏核禾貉阂河涸赫褐鹤贺吓诃劾壑嗬阖纥曷盍颌蚵翮"},
    {"hei","嘿黑"},
    {"hen","很狠恨痕"},
    {"heng","哼亨横衡恒蘅珩桁"},
    {"hong","红虹鸿洪宏弘轰哄烘黉訇讧荭蕻薨闳泓"},
    {"hou","后喉侯猴吼厚候堠後逅瘊篌糇鲎骺"},
    {"hu","呼虎唬护互沪户乎忽瑚壶葫胡蝴狐糊湖弧冱唿囫核岵猢怙惚浒滹琥槲轷觳烀煳戽扈祜瓠鹄鹕鹱虍笏醐斛鹘"},
    {"hua","花滑画划化话哗华猾骅桦砉铧"},
    {"huai","坏槐怀淮踝徊"},
    {"huan","欢环患唤桓还缓换痪豢焕涣宦幻郇奂垸萑擐圜獾洹浣漶寰逭缳锾鲩鬟"},
    {"huang","荒慌黄皇凰磺蝗簧惶煌晃幌恍谎隍徨湟潢遑璜肓癀蟥篁鳇"},
    {"hui","会灰挥辉绘烩汇讳诲徽恢蛔回毁悔慧彗卉惠晦贿秽溃诙茴荟蕙咴哕喙隳洄浍缋桧晖恚虺蟪麾"},
    {"hun","魂浑混荤昏婚诨馄阍溷珲"},
    {"huo","和活货伙火获或惑霍豁祸劐藿攉嚯夥钬锪镬耠蠖"},
    {"ji","给击几脊己基集及急机技际冀季畸稽积箕肌饥迹激讥鸡姬计记既忌妓继纪藉绩缉吉极棘辑籍疾汲即嫉级挤蓟伎祭剂悸济寄寂奇系丌亟乩剞佶偈墼芨芰荠萁蒺蕺掎叽咭哜唧岌嵴洎骥畿玑楫殛戟戢赍觊犄齑矶羁嵇稷瘠虮笈笄暨跻跽霁鲚鲫髻麂"},
    {"jia","家加价架夹佳假贾甲嘉驾嫁枷荚颊钾稼茄嘏伽郏葭岬浃迦珈戛胛恝铗铪镓痂瘕袷蛱笳袈跏"},
    {"jian","歼监坚尖笺间煎兼肩艰奸捡简缄茧检柬碱硷拣俭剪减荐槛鉴践贱见键箭件健舰剑饯渐溅涧建僭谏谫谮菅蒹搛囝湔蹇謇缣枧楗戋戬牮犍毽腱睑锏鹣裥笕翦趼踺鲣鞯"},
    {"jiang","讲将江疆降蒋僵姜浆桨奖匠酱强茳洚绛缰犟礓耩糨豇"},
    {"jiao","焦胶交较叫郊浇骄娇觉嚼蕉椒礁搅铰矫侥脚狡角饺缴绞剿教酵轿窖校佼僬艽茭挢噍峤徼姣敫皎鹪蛟醮跤鲛"},
    {"jie","洁结解姐节桔杰借介戒揭接皆秸街阶截劫捷睫竭藉芥界疥诫届偈讦诘卩拮喈嗟婕孑桀碣锴疖颉蚧羯鲒骱"},
    {"jin","尽劲巾仅谨进筋斤金今津晋禁近襟紧锦靳烬浸卺荩堇噤馑廑妗缙瑾槿赆觐衿矜"},
    {"jing","竟竞净劲荆晶鲸京井警景惊静境精粳经兢茎睛敬镜径痉靖刭儆阱陉菁獍憬泾迳弪婧肼胫腈旌靓"},
    {"jiong","炯窘冂迥扃"},
    {"jiou","九酒就疚揪究纠玖韭久灸厩救旧臼舅咎僦啾阄柩桕鸠鹫赳鬏"},
    {"ju","据巨具距句惧炬剧车柜鞠拘狙疽居驹菊局咀矩举沮聚拒踞锯俱倨讵苣苴莒掬遽屦琚枸椐榘榉橘犋飓钜锔窭裾趄醵踽龃雎瞿鞫"},
    {"juan","卷绢圈捐鹃娟倦眷鄄狷涓桊蠲锩镌隽"},
    {"jue","爵觉嚼脚角抉掘倔决诀撅攫绝厥劂谲矍堀蕨噘崛獗孓珏桷橛爝镢蹶觖"},
    {"jun","军君龟均菌钧峻俊竣浚郡骏捃皲筠麇"},
    {"ka","卡喀咯咖胩咔佧"},
    {"kai","开揩凯慨楷垲剀锎铠锴忾恺蒈"},
    {"kan","看砍堪刊坎槛勘龛戡侃瞰莰阚"},
    {"kang","抗炕扛糠康慷亢钪闶伉"},
    {"kao","靠考烤拷栲犒尻铐"},
    {"ke","可克棵科颗刻课客壳渴苛柯磕咳坷恪岢蝌缂蚵轲窠钶氪颏瞌锞稞珂髁疴嗑溘骒铪"},
    {"ken","肯啃恳垦裉"},
    {"keng","坑吭铿硁阬"},
    {"kong","空孔控恐倥崆箜"},
    {"kou","口扣抠叩寇蔻芤眍筘"},
    {"ku","哭库苦枯裤窟酷刳骷喾堀绔"},
    {"kua","跨垮挎夸胯侉"},
    {"kuai","快块筷会侩脍哙蒯郐狯"},
    {"kuan","宽款髋"},
    {"kuang","矿筐狂框况旷匡眶诳邝纩夼诓圹贶哐"},
    {"kui","亏愧奎窥溃葵魁馈盔傀岿匮愦揆睽跬聩篑喹逵暌蒉悝喟馗蝰隗夔"},
    {"kun","捆困昆坤鲲锟髡琨醌阃悃"},
    {"kuo","阔扩括廓蛞"},
    {"la","垃拉啦剌辣落腊蜡喇旯砬邋瘌"},
    {"lai","来莱赖籁癞睐徕涞濑赉铼崃"},
    {"lan","蓝兰篮栏拦婪览揽懒烂岚缆滥榄褴阑澜谰斓漤罱镧"},
    {"lang","琅狼廊郎朗浪螂榔莨蒗啷阆锒稂"},
    {"lao","老劳牢捞佬姥酪烙涝落络唠崂栳铑铹痨耢醪"},
    {"le","了乐勒叻仂泐鳓"},
    {"lei","类泪累雷蕾羸镭擂磊垒肋儡勒嘞诔嫘缧檑耒酹"},
    {"leng","冷棱愣楞塄"},
    {"li","里李力离利立丽例厘吏历利厉莉荔隶栗励砾犁黎鲤篱漓傈俐痢粒沥璃哩鬲俪俚郦坜苈莅蓠藜呖唳喱猁溧澧逦娌嫠骊缡枥栎轹砺砬詈罹锂鹂疠疬蛎蜊蠡笠篥粝醴跞雳鲡鳢黧"},
    {"lia","俩"},
    {"lian","联莲连怜涟帘恋练脸炼链敛镰廉琏楝殓裢裣蔹奁潋濂臁蠊鲢"},
    {"liang","俩两良量亮谅凉辆晾粮梁粱踉椋墚莨魉"},
    {"liao","了聊料辽疗撂廖蓼寥潦撩僚燎镣尥嘹獠寮钌鹩"},
    {"lie","列劣烈猎裂冽咧洌埒捩趔躐鬣"},
    {"lin","林琳临邻拎霖鳞淋凛赁吝磷蔺嶙啉廪懔檩辚膦瞵粼躏麟"},
    {"ling","另令灵玲零岭领龄凌伶铃陵棱菱羚苓翎绫瓴泠呤蛉棂柃鲮酃"},
    {"liu","六陆刘浏留流柳琉榴硫瘤溜馏骝遛碌绺熘锍镏鹨鎏"},
    {"lo","咯"},
    {"long","龙聋笼垄弄隆拢陇泷窿垅茏咙珑栊胧砻癃"},
    {"lou","楼娄搂篓漏陋露嵝髅偻蒌喽镂瘘耧蝼"},
    {"lu","六卢芦颅庐炉掳卤陆路录鲁赂绿虏麓碌露潞禄戮垆撸噜泸渌漉逯璐栌橹轳辂辘贲胪镥鸬鹭簏舻鲈"},
    {"lv","吕绿铝侣驴旅履屡缕虑氯律率滤偻捋闾榈稆褛"},
    {"lve","掠略锊"},
    {"luan","峦卵乱孪娈栾鸾銮挛滦脔"},
    {"lun","仑轮论抡纶伦囵沦"},
    {"luo","罗逻洛络落裸骆铬锣箩萝咯烙骡螺捋摞倮蠃荦猡泺漯椤脶硌镙瘰雒"},
    {"m","呒"},
    {"ma","马妈吗码蚂麻骂嘛玛摩抹唛犸嬷杩蟆"},
    {"mai","买卖麦埋迈脉劢荬霾"},
    {"man","埋慢满蛮漫蔓曼瞒馒谩墁幔缦熳镘颟螨鳗鞔"},
    {"mang","忙芒茫盲氓莽邙漭硭蟒"},
    {"mao","毛猫卯茂冒矛茅贸貌锚帽铆髦袤茆峁泖瑁昴牦耄旄懋瞀蝥蟊"},
    {"me","么"},
    {"mei","没美每妹眉玫枚梅酶霉煤昧媒镁寐袂魅媚莓嵋猸浼湄楣镅鹛"},
    {"men","门闷们扪焖懑钔"},
    {"meng","孟梦猛盟萌蒙檬锰勐甍瞢懵朦礞虻蜢蠓艋艨"},
    {"mi","米迷密秘觅谜蜜幂泌弥眯醚靡糜谧蘼咪嘧猕汨宓弭麋纟脒祢敉糸縻冖芈"},
    {"mian","免面眠勉棉绵冕娩缅腼沔渑湎宀眄"},
    {"miao","苗秒妙庙喵描瞄藐渺眇缈缪淼邈杪鹋"},
    {"mie","乜咩灭蔑蠛篾"},
    {"min","民皿闵敏悯闽苠岷抿泯缗玟珉愍黾鳘"},
    {"ming","明名命冥鸣铭茗螟酩溟暝瞑"},
    {"miu","谬缪"},
    {"mo","没末莫万脉抹陌魔模摸磨墨默摩摹膜漠寞沫麽蓦馍殁谟茉秣蘑镆嫫瘼耱貊貘"},
    {"mou","某牟谋眸缪侔哞蛑鍪"},
    {"mu","目木母亩牧模牟拇牡墓暮幕募慕姆睦穆仫坶苜沐毪钼"},
    {"na","拿那哪钠呐娜纳捺肭镎衲"},
    {"nai","乃奈奶耐氖佴鼐艿萘柰"},
    {"nan","男南难喃囡楠腩蝻赧"},
    {"nang","囊攮囔馕曩"},
    {"nao","闹脑恼挠垴孬淖呶猱瑙硇铙蛲"},
    {"ne","呢讷哪"},
    {"nei","内馁"},
    {"nen","恁嫩"},
    {"neng","能"},
    {"ng","嗯"},
    {"ni","你尼逆泥妮伲呢匿昵怩拟霓倪溺腻坭猊祢慝睨铌鲵"},
    {"nian","年廿念拈碾撵捻埝辇黏鲇鲶蔫"},
    {"niang","娘酿"},
    {"niao","鸟尿茑嬲脲袅"},
    {"nie","捏聂乜涅孽镊镍蹑陧蘖嗫颞臬啮"},
    {"nin","您"},
    {"ning","宁咛拧柠狞凝泞佞甯聍"},
    {"niu","牛扭纽妞忸钮拗狃"},
    {"nong","农弄侬浓哝脓"},
    {"nou","耨"},
    {"nu","奴努怒弩胬孥驽"},
    {"nv","女恧钕衄"},
    {"nve","虐疟"},
    {"nuan","暖"},
    {"nuo","诺喏挪娜懦糯傩搦锘"},
    {"o","哦喔噢"},
    {"ou","呕讴怄偶欧鸥殴沤区瓯藕耦"},
    {"pa","扒怕啪趴爬帕琶葩耙杷筢"},
    {"pai","拍排牌徘湃派迫俳哌蒎"},
    {"pan","盘判盼胖畔潘叛番磐攀拚蟠蹒泮贲袢襻丬爿"},
    {"pang","乓旁庞胖彷滂逄螃膀磅镑耪"},
    {"pao","跑咆泡抛刨炮袍匏狍庖脬疱"},
    {"pei","呸胚培裴陪赔配佩沛辔帔旆锫醅霈"},
    {"pen","盆喷湓"},
    {"peng","朋捧碰抨烹砰蓬彭棚硼怦堋嘭澎篷膨鹏蟛"},
    {"pi","皮匹否屁丕批披辟劈琵啤脾疲坯砒霹毗痞譬仳陂陴邳郫圮埤鼙芘擗噼庀淠媲纰枇甓睥罴铍癖裨疋蚍蜱貔"},
    {"piao","票朴飘漂瓢剽嘌嫖骠缥殍瞟螵"},
    {"pie","丿撇瞥苤氕"},
    {"pin","品贫拼频聘姘嫔榀牝颦"},
    {"ping","乒平屏评凭苹萍坪瓶冯俜娉枰鲆"},
    {"po","迫泊叵坡陂破泼颇婆魄朴粕鄱珀攴攵钋钷笸繁"},
    {"pou","剖裒掊"},
    {"pu","仆扑朴普菩铺圃暴莆葡蒲埔浦脯堡谱曝瀑匍噗溥濮璞氆镤镨蹼"},
    {"qi","其起气期七奇器企骑岂启齐旗欺栖戚妻弃汽泣乞岐契砌迄凄漆沏棋缉歧畦祺憩崎脐稽祈祁柒汔淇骐绮琪琦杞碛颀蛴蜞綦綮蹊鳍麒亓桤槭耆欹讫亟屺俟圻芑芪荠萋葺蕲嘁"},
    {"qia","恰掐卡洽葜袷髂"},
    {"qian","前钱千欠签牵扦遣浅谴铅迁谦歉茜纤仟钳潜堑钎嵌倩慊佥阡芊乾黔芡荨掮岍悭骞搴褰缱椠犍肷愆钤虔箝羟"},
    {"qiang","强抢墙枪呛腔蔷羌锵镪襁跄丬戕嫱樯戗炝蜣羟"},
    {"qiao","桥巧敲翘悄瞧乔侨峭俏窍壳跷橇诮樵谯荞锹鞘撬雀劁峤愀憔缲硗铫鞒"},
    {"qie","且切妾茄怯窃惬挈锲慊伽郄箧趄"},
    {"qin","亲秦琴勤芹钦擒禽寝沁侵吣嗪衾噙芩揿廑檎锓矜覃螓"},
    {"qing","请情轻青庆清晴倾卿顷擎蜻氢磬罄氰箐綮謦鲭黥苘圊檠锖"},
    {"qiong","穷琼穹邛茕蛩筇跫銎"},
    {"qiu","求球秋邱糗丘囚酋蚯裘鳅逑遒楸龟赇虬仇蝤泅俅巯犰湫"},
    {"qu","去区取趋曲娶躯屈趣驱渠蛆龋戌诎劬凵苣蕖蘧岖衢阒璩觑氍朐祛磲鸲癯蛐蠼麴瞿黢"},
    {"quan","全权圈劝泉痊犬券荃诠绻蜷铨拳颧醛犭悛鬈辁畎筌"},
    {"que","却缺确雀瘸阙鹊榷阕炔悫"},
    {"qui","鼽"},
    {"qun","群裙逡麇"},
    {"ran","然染燃冉苒蚺髯"},
    {"rang","让嚷壤攘瓤禳穰"},
    {"rao","绕饶扰娆荛桡"},
    {"re","热惹喏"},
    {"ren","人任忍认仁刃韧壬妊纫亻仞荏葚饪轫稔衽"},
    {"reng","仍扔"},
    {"ri","日"},
    {"rong","荣容蓉融熔溶绒戎茸冗嵘狨榕肜蝾"},
    {"rou","肉柔揉糅蹂鞣"},
    {"ru","如入乳茹汝儒辱褥濡蠕孺襦蓐薷嚅洳溽缛铷颥"},
    {"ruan","软阮朊"},
    {"rui","瑞锐睿蕊芮蕤枘蚋"},
    {"run","润闰"},
    {"ruo","若弱偌箬"},
    {"sa","撒萨洒卅仨脎飒"},
    {"sai","赛塞腮鳃塞噻"},
    {"san","三散伞叁馓毵糁"},
    {"sang","桑丧嗓搡磉颡"},
    {"sao","扫骚嫂臊搔埽缫瘙鳋"},
    {"se","色涩瑟啬塞铯穑"},
    {"sen","森"},
    {"seng","僧"},
    {"sha","啥杀傻莎沙纱煞杉厦刹砂唼歃铩痧裟霎鲨"},
    {"shai","晒筛色"},
    {"shan","山删珊扇闪陕掺单衫杉汕擅赡膳善苫煽缮栅剡讪鄯埏芟彡潸姗嬗骟膻禅钐疝蟮舢跚鳝髟"},
    {"shang","上商尚伤赏晌裳殇垧墒绱熵觞"},
    {"shao","少烧稍绍邵勺哨鞘梢韶捎芍劭苕潲杓蛸筲艄"},
    {"she","设社射摄蛇舌舍奢涉赊赦慑折厍佘揲猞滠歙畲铊麝"},
    {"shei","谁"},
    {"shen","神深身什参甚伸肾慎沈审婶砷申呻娠绅渗诜谂莘葚哂渖椹胂矧蜃"},
    {"sheng","生省升声乘胜圣绳盛剩甥牲嵊渑晟眚笙"},
    {"shi","是时事使市十师世实失施士石氏拾示什食式室视试似始湿诗尸适仕释饰蚀识史矢屎驶拭匙狮柿虱誓逝势嗜噬侍恃殖峙谥埘莳蓍弑饣轼贳炻礻铈铊螫舐筮酾豕鲥鲺"},
    {"shou","收受手瘦兽首守寿授售扌狩绶艏"},
    {"shu","数书输蔬鼠属术叔树熟梳殊暑抒舒述淑疏赎孰薯曙署枢蜀黍束戍竖墅庶漱恕丨倏塾菽摅沭澍姝纾毹腧殳镯秫疋"},
    {"shua","刷耍唰"},
    {"shuai","帅甩衰率摔蟀"},
    {"shuan","栓涮拴闩"},
    {"shuang","爽霜双泷孀"},
    {"shui","谁水睡税说氵"},
    {"shun","顺瞬舜吮"},
    {"shuo","说硕朔烁数蒴搠妁槊铄"},
    {"si","死四思私司丝似斯撕嘶肆寺嗣伺饲巳厮俟兕厶咝饣汜泗澌姒驷缌祀锶鸶耜蛳笥"},
    {"song","送宋松颂讼诵耸怂凇菘崧嵩忪悚淞竦"},
    {"sou","搜艘嗽叟薮嗖擞嗾馊溲飕瞍锼螋"},
    {"su","素速苏诉宿肃俗酥塑溯缩夙粟僳谡蔌嗉愫涑簌觫稣"},
    {"suan","算酸蒜狻"},
    {"sui","岁虽随碎穗遂隧隋祟绥髓谇荽濉邃尿燧眭睢"},
    {"sun","孙损笋荪狲飧榫隼"},
    {"suo","所索锁缩唢嗦莎蓑梭唆琐嗍娑桫挲睃羧"},
    {"ta","他她它塌塔獭挞蹋踏拓闼溻漯遢榻沓铊趿鳎"},
    {"tai","太台泰抬态胎苔酞汰邰薹骀肽炱钛跆鲐"},
    {"tan","谈弹贪摊谭瘫滩探叹炭痰坛潭坦毯檀坍袒碳郯澹昙赕忐钽锬镡覃"},
    {"tang","汤唐塘搪堂棠膛糖倘躺淌趟烫溏瑭傥镗饧樘铛帑铴耥螗螳羰醣"},
    {"tao","套逃涛掏滔桃淘陶啕洮萄韬绦讨叨焘饕鼗"},
    {"te","特忒忑铽"},
    {"teng","疼藤腾誊滕"},
    {"ti","提体题踢替蹄梯剃屉倜剔锑啼嚏惕涕荑悌逖绨缇鹈裼醍"},
    {"tian","天田添填甜恬舔腆掭忝阗殄畋钿锘"},
    {"tiao","条跳调挑迢眺佻窕粜蜩笤龆苕祧铫鲦髫"},
    {"tie","贴铁帖萜锇餮"},
    {"ting","听挺停婷厅烃庭廷亭艇汀莛葶梃铤蜓霆"},
    {"tong","同童痛通统桶捅筒桐酮瞳铜彤佟恸潼僮仝垌茼嗵峒砼"},
    {"tou","头偷投透钭骰亠"},
    {"tu","图土吐兔秃涂途突徒凸屠堍荼菟钍酴"},
    {"tuan","团湍抟彖疃"},
    {"tui","退推腿忒颓蜕褪煺"},
    {"tun","吞屯臀豚囤褪氽饨暾"},
    {"tuo","托脱拖陀妥驮驼唾椭坨拓鸵乇沱说砣铊佗庹柝柁橐箨酡跎鼍"},
    {"wa","哇瓦挖娃洼蛙袜佤娲腽"},
    {"wai","外歪崴"},
    {"wan","玩完万晚弯碗湾挽顽丸蔓豌烷皖惋宛婉腕剜芄莞菀纨绾琬脘畹蜿鞔"},
    {"wang","网王往望汪亡枉旺忘妄罔尢惘辋魍"},
    {"wei","为喂未威微危围味唯畏胃惟维韦苇潍违慰卫萎委伟伪尾纬蔚巍桅魏位渭谓尉偎诿隈隗圩葳薇囗帏帷崴嵬猥猬闱沩洧涠逶娓玮韪軎炜煨痿艉鲔"},
    {"wen","问文闻温吻稳纹紊蚊瘟刎阌汶玟璺雯"},
    {"weng","翁嗡瓮蓊蕹"},
    {"wo","我喔窝蜗卧握涡斡挝沃倭莴幄渥肟硪龌"},
    {"wu","无五吴武物污舞务午屋勿乌悟误呜恶吾毋雾诬巫捂侮伍钨芜梧坞戊晤兀仵阢邬圬芴唔庑怃忤浯寤迕妩婺骛杌牾於焐鹉鹜痦蜈鋈鼯"},
    {"xi","希西洗喜系昔吸牺稀熙习息悉膝夕惜析熄犀溪晰嘻锡汐袭席媳铣隙戏细僖奚兮硒矽檄蟋淅蜥螅屣嬉匚烯隰郗茜菥葸蓰唏徙饩阋浠玺樨曦觋欷歙熹禊禧皙穸裼舄舾羲粞翕醯蹊鼷"},
    {"xia","下夏吓瞎霞侠辖虾峡狭匣暇厦呷狎遐瑕柙硖罅黠"},
    {"xian","铣洗掀锨先仙鲜纤咸贤衔舷闲涎弦嫌显险现献县腺馅羡宪陷限线冼苋莶藓岘猃暹娴氙燹祆鹇痃痫蚬筅籼酰跣跹霰"},
    {"xiang","想向象像香相箱响项降乡祥详享厢镶襄湘翔巷橡芗葙饷庠骧缃蟓鲞飨"},
    {"xiao","小笑校肖孝销效哓萧硝逍霄削哮嚣消宵淆晓啸潇骁绡枭枵蛸筱箫魈"},
    {"xie","写些谢鞋血协邪斜歇挟械卸携蟹懈泄泻偕亵胁谐蝎解屑叶楔榭勰燮薤撷獬廨渫瀣邂绁缬榍颉躞鲑骱"},
    {"xin","新心信欣芯辛薪鑫馨歆衅锌忻囟莘忄昕镡"},
    {"xing","行性型形星姓兴醒幸腥猩惺刑邢省杏陉荇荥擤饧悻硎"},
    {"xiong","雄熊凶兄胸匈汹芎"},
    {"xiu","修秀休宿绣袖羞嗅锈朽咻臭岫馐庥溴鸺貅髹"},
    {"xu","需须徐许虚嘘蓄叙序酗旭墟絮戌畜胥恤婿绪续吁诩勖圩蓿洫浒溆顼栩煦盱糈醑"},
    {"xuan","选轩玄喧炫宣旋悬券癣眩绚萱揎泫渲漩璇儇谖楦暄煊碹铉镟"},
    {"xue","学血雪薛穴靴削谑噱泶踅鳕"},
    {"xun","寻训讯熏勋循旬询巡迅荀殉汛驯浚逊荨薰峋徇巽郇埙蕈獯恂洵浔曛窨醺鲟"},
    {"ya","呀压牙丫亚雅鸭押哑涯鸦崖芽蚜衙讶轧伢垭揠岈迓娅琊桠氩砑睚痖疋"},
    {"yan","眼烟言演严铅燕艳彦炎颜盐岩验焰宴焉咽阉淹研雁掩沿阎兖蜒奄妍俨嫣衍堰晏胭厌砚唁谚殷厣赝剡偃讠谳阽郾鄢芫菸崦恹闫阏湮滟琰檐腌焱罨筵酽魇餍鼹鼽"},
    {"yang","样养扬杨阳羊洋氧痒殃央鸯秧佯疡仰漾徉怏泱炀烊恙蛘鞅"},
    {"yao","要药咬摇腰妖邀谣瑶尧窑姚耀遥侥肴钥夭幺舀曜啮疟爻珧杳轺吆崤崾徭铫鹞窈繇鳐"},
    {"ye","也页叶爷夜野咽业曳腋椰噎耶冶揶掖液邪盅靥谒邺琊晔烨铘"},
    {"yi","一以衣易已意毅忆义益饴艺遗医艾移仪疫亦乙抑怿怡椅蚁倚矣尾壹依伊颐夷胰疑沂宜姨揖铱彝邑屹亿役臆逸肄裔溢诣议谊译异翼翌绎弈奕挹弋呓咦咿噫峄嶷猗悒漪迤驿缢殪轶贻欹旖熠刈劓佚佾诒阝圯埸懿苡荑薏眙钇铊镒镱痍瘗癔翊衤蜴舣羿翳酏黟"},
    {"yin","因音阴银引隐印淫饮殷吟茵荫寅尹垠姻蚓胤霪廴堙鄞茚吲喑狺夤洇湮氤铟瘾窨龈"},
    {"ying","应赢硬影英莹萤营颖映荧盈迎莺蝇鹰樱婴缨颍嬴罂瑛膺郢茔荥萦蓥撄嘤滢潆瀛璎楹媵鹦瘿"},
    {"yo","哟育唷"},
    {"yong","用永拥勇佣咏泳庸踊雍蛹涌恿臃痈俑甬壅墉喁慵邕镛鳙饔"},
    {"you","有又友右幼游优油攸幽悠忧诱尤邮犹铀酉尢佑釉卣侑莠莜莸呦囿宥柚猷牖铕疣蚰蚴蝣蝤繇鱿黝鼬"},
    {"yu","与雨于鱼余语羽玉域育郁俞遇喻愉渔预隅予娱迂淤盂逾渝愚舆屿禹浴宇芋吁峪御愈欲狱豫誉寓豫裕妪妤纡瑜蔚尉驭禺毓伛俣谀谕萸竽臾虞菀榆蓣揄圄圉嵛狳饫馀庾阈鬻昱觎腴欤於煜熨燠肀聿钰鹆鹬瘐瘀窬窳蜮蝓舁雩龉"},
    {"yuan","远元园员院袁原冤媛圆愿怨源缘苑鸳渊猿垣援辕塬芫掾圜沅瑗橼爰眢鸢螈箢鼋"},
    {"yue","月约越曰乐跃阅岳粤悦说钥龠哕瀹栎樾刖钺"},
    {"yun","晕运韵云芸孕耘允蕴匀陨酝郧郓员狁恽愠纭韫殒昀氲熨筠"},
    {"za","咋杂砸扎咂匝拶"},
    {"zai","在再栽宰载仔崽哉灾甾"},
    {"zan","咱攒赞瓒暂簪糌昝趱錾拶"},
    {"zang","脏葬藏赃驵臧"},
    {"zao","早造枣澡糟灶燥遭蚤躁噪凿皂藻唣"},
    {"ze","则泽责择咋仄赜啧帻迮昃笮箦舴"},
    {"zei","贼"},
    {"zen","怎谮"},
    {"zeng","增憎曾赠缯甑罾锃"},
    {"zha","炸诈咋渣眨乍札轧铡闸栅榨喳扎柞揸吒查咤哳喋楂砟痄蚱龃齄"},
    {"zhai","摘宅窄债寨翟择斋债寨砦瘵"},
    {"zhan","战站占展斩辗崭颤詹粘沾瞻毡盏蘸栈湛绽谵搌骣旃"},
    {"zhang","张掌涨章彰漳帐杖胀丈账仗长樟瘴障仉鄣幛嶂獐嫜璋蟑"},
    {"zhao","找照招朝赵昭着沼罩兆肇召爪诏啁棹钊笊"},
    {"zhe","这着者折哲蛰遮浙辙锗蔗谪摺柘辄磔鹧褶蜇赭"},
    {"zhen","真镇贞针侦珍震振阵帧诊朕祯斟圳甄砧臻枕疹蓁浈溱缜桢椹榛轸赈胗畛稹鸩箴"},
    {"zheng","正整政争怔郑证拯症诤蒸挣睁征狰峥徵钲铮筝鲭"},
    {"zhi","之只纸志至致置汁质织职直支植旨掷识芝痣殖执值挚枝吱蜘知肢脂侄址指止趾帜峙制智秩稚炙痔滞治窒卮陟郅埴芷栉枳栀桎轵轾贽胝膣祉祗黹雉鸷蛭絷酯跖踬踯豸觯摭帙徵夂忮彘咫骘"},
    {"zhong","中种钟重衷终肿仲众盅忠冢忪锺螽舯踵"},
    {"zhou","周舟州洲宙昼粥轴帚咒皱肘胄骤纣诌荮啁妯绉碡籀繇酎"},
    {"zhu","住朱猪诸主注祝驻珠株蛛逐竹烛煮诛拄瞩嘱著柱助蛀贮铸筑丶伫属术侏邾苎茱洙渚潴杼槠橥炷铢疰瘃褚竺箸舳翥躅麈"},
    {"zhua","抓爪挝"},
    {"zhuai","拽转"},
    {"zhuan","转赚传专砖撰篆啭馔沌颛"},
    {"zhuang","装状撞壮幢桩庄妆奘戆"},
    {"zhui","追坠椎锥赘缀惴骓缒隹"},
    {"zhun","准谆饨肫窀"},
    {"zhuo","桌捉拙卓着琢茁酌啄灼浊倬诼擢浞涿濯焯禚斫镯"},
    {"zi","子自字资兹咨紫仔姿渍恣滋淄籽吱孜滓谘茈嵫姊孳缁梓辎赀眦锱秭耔笫粢趑觜訾龇鲻髭"},
    {"zong","总宗宗综纵踪鬃棕偬枞腙粽"},
    {"zou","走揍邹奏诹陬鄹驺鲰"},
    {"zu","组租足卒族祖诅阻俎菹镞"},
    {"zuan","钻纂攥缵躜"},
    {"zui","嘴醉最罪蕞觜"},
    {"zun","尊遵撙樽鳟"},
    {"zuo","做作坐座昨左撮琢佐柞阼唑嘬怍胙祚砟酢"},
    {NULL, NULL}
};
#endif


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * lv_ime_pinyin_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


/*=====================
 * Setter functions
 *====================*/

/**
 * Set the keyboard of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method keyboard
 */
void lv_ime_pinyin_set_keyboard(lv_obj_t * obj, lv_obj_t * kb)
{
    if(kb) {
        LV_ASSERT_OBJ(kb, &lv_keyboard_class);
    }

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->kb = kb;
    lv_obj_set_parent(obj, lv_obj_get_parent(kb));
    lv_obj_set_parent(pinyin_ime->cand_panel, lv_obj_get_parent(kb));
    lv_obj_add_event_cb(pinyin_ime->kb, lv_ime_pinyin_kb_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_align_to(pinyin_ime->cand_panel, pinyin_ime->kb, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @param dict pointer to a Pinyin input method dictionary
 */
void lv_ime_pinyin_set_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    init_pinyin_dict(obj, dict);
}

/**
 * Set mode, 26-key input(k26) or 9-key input(k9).
 * @param obj  pointer to a Pinyin input method object
 * @param mode   the mode from 'lv_keyboard_mode_t'
 */
void lv_ime_pinyin_set_mode(lv_obj_t * obj, lv_ime_pinyin_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    LV_ASSERT_OBJ(pinyin_ime->kb, &lv_keyboard_class);

    pinyin_ime->mode = mode;

#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_k9_init_data(obj);
        lv_keyboard_set_map(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1, (const char **)lv_btnm_def_pinyin_k9_map,
                            (const lv_btnmatrix_ctrl_t *)default_kb_ctrl_k9_map);
        lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1);
    }
#endif
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin IME object
 * @return     pointer to the Pinyin IME keyboard
 */
lv_obj_t * lv_ime_pinyin_get_kb(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->kb;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method candidate panel
 */
lv_obj_t * lv_ime_pinyin_get_cand_panel(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->cand_panel;
}

/**
 * Set the dictionary of Pinyin input method.
 * @param obj  pointer to a Pinyin input method object
 * @return     pointer to the Pinyin input method dictionary
 */
lv_pinyin_dict_t * lv_ime_pinyin_get_dict(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->dict;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ime_pinyin_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM + 3); btnm_i++) {
        if(btnm_i == 0) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "<";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = ">";
        }
        else if(btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 2)) {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "";
        }
        else {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
        }
    }

    pinyin_ime->mode = LV_IME_PINYIN_MODE_K26;
    pinyin_ime->py_page = 0;
    pinyin_ime->ta_count = 0;
    pinyin_ime->cand_num = 0;
    lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
    lv_memset_00(pinyin_ime->py_num, sizeof(pinyin_ime->py_num));
    lv_memset_00(pinyin_ime->py_pos, sizeof(pinyin_ime->py_pos));

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(55));
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);

#if LV_IME_PINYIN_USE_DEFAULT_DICT
    init_pinyin_dict(obj, lv_ime_pinyin_def_dict);
#endif

    /* Init pinyin_ime->cand_panel */
    pinyin_ime->cand_panel = lv_btnmatrix_create(lv_obj_get_parent(obj));
    lv_btnmatrix_set_map(pinyin_ime->cand_panel, (const char **)lv_btnm_def_pinyin_sel_map);
    lv_obj_set_size(pinyin_ime->cand_panel, LV_PCT(100), LV_PCT(5));
    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_btnmatrix_set_one_checked(pinyin_ime->cand_panel, true);
    lv_obj_clear_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_all(pinyin_ime->cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_base_dir(pinyin_ime->cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* event handler */
    lv_obj_add_event_cb(pinyin_ime->cand_panel, lv_ime_pinyin_cand_panel_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_add_event_cb(obj, lv_ime_pinyin_style_change_event, LV_EVENT_STYLE_CHANGED, NULL);

#if LV_IME_PINYIN_USE_K9_MODE
    pinyin_ime->k9_input_str_len = 0;
    pinyin_ime->k9_py_ll_pos = 0;
    pinyin_ime->k9_legal_py_count = 0;
    lv_memset_00(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);

    pinyin_k9_init_data(obj);

    _lv_ll_init(&(pinyin_ime->k9_legal_py_ll), sizeof(ime_pinyin_k9_py_str_t));
#endif
}


static void lv_ime_pinyin_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(lv_obj_is_valid(pinyin_ime->kb))
        lv_obj_del(pinyin_ime->kb);

    if(lv_obj_is_valid(pinyin_ime->cand_panel))
        lv_obj_del(pinyin_ime->cand_panel);
}


static void lv_ime_pinyin_kb_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    static const char * k9_py_map[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
#endif

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id  = lv_btnmatrix_get_selected_btn(kb);
        if(btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char * txt = lv_btnmatrix_get_btn_text(kb, lv_btnmatrix_get_selected_btn(kb));
        if(txt == NULL) return;

#if LV_IME_PINYIN_USE_K9_MODE
        if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
            lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
            uint16_t tmp_btn_str_len = strlen(pinyin_ime->input_char);
            if((btn_id >= 16) && (tmp_btn_str_len > 0) && (btn_id < (16 + LV_IME_PINYIN_K9_CAND_TEXT_NUM))) {
                tmp_btn_str_len = strlen(pinyin_ime->input_char);
                lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
                strcat(pinyin_ime->input_char, txt);
                pinyin_input_proc(obj);

                for(int index = 0; index < (pinyin_ime->ta_count + tmp_btn_str_len); index++) {
                    lv_textarea_del_char(ta);
                }

                pinyin_ime->ta_count = tmp_btn_str_len;
                pinyin_ime->k9_input_str_len = tmp_btn_str_len;
                lv_textarea_add_text(ta, pinyin_ime->input_char);

                return;
            }
        }
#endif

        if(strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
            pinyin_ime_clear_data(obj);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
            // del input char
            if(pinyin_ime->ta_count > 0) {
                if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                    pinyin_ime->input_char[pinyin_ime->ta_count - 1] = '\0';
#if LV_IME_PINYIN_USE_K9_MODE
                else
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count - 1] = '\0';
#endif

                pinyin_ime->ta_count = pinyin_ime->ta_count - 1;
                if(pinyin_ime->ta_count <= 0) {
                    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
#if LV_IME_PINYIN_USE_K9_MODE
                    lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
                    strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
                    strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
#endif
                }
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                    pinyin_input_proc(obj);
                }
#if LV_IME_PINYIN_USE_K9_MODE
                else if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
                    pinyin_ime->k9_input_str_len = strlen(pinyin_ime->input_char) - 1;
                    pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
                    pinyin_k9_fill_cand(obj);
                    pinyin_input_proc(obj);
                }
#endif
            }
        }
        else if((strcmp(txt, "ABC") == 0) || (strcmp(txt, "abc") == 0) || (strcmp(txt, "1#") == 0)) {
            pinyin_ime->ta_count = 0;
            lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
            return;
        }
        else if(strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) {
            if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) {
                lv_ime_pinyin_set_mode((lv_obj_t *)pinyin_ime, LV_IME_PINYIN_MODE_K9);
            }
            else {
                lv_ime_pinyin_set_mode((lv_obj_t *)pinyin_ime, LV_IME_PINYIN_MODE_K26);
                lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
            pinyin_ime_clear_data(obj);
        }
        else if(strcmp(txt, LV_SYMBOL_OK) == 0) {
            pinyin_ime_clear_data(obj);
        }
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) && ((txt[0] >= 'a' && txt[0] <= 'z') || (txt[0] >= 'A' &&
                                                                                                      txt[0] <= 'Z'))) {
            strcat(pinyin_ime->input_char, txt);
            pinyin_input_proc(obj);
            pinyin_ime->ta_count++;
        }
#if LV_IME_PINYIN_USE_K9_MODE
        else if((pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) && (txt[0] >= 'a' && txt[0] <= 'z')) {
            for(uint16_t i = 0; i < 8; i++) {
                if((strcmp(txt, k9_py_map[i]) == 0) || (strcmp(txt, "abc ") == 0)) {
                    if(strcmp(txt, "abc ") == 0)    pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]) + 1;
                    else                            pinyin_ime->k9_input_str_len += strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;

                    break;
                }
            }
            pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
            pinyin_k9_fill_cand(obj);
            pinyin_input_proc(obj);
        }
        else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) {
            pinyin_k9_cand_page_proc(obj, 0);
        }
        else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) {
            pinyin_k9_cand_page_proc(obj, 1);
        }
#endif
    }
}


static void lv_ime_pinyin_cand_panel_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cand_panel = lv_event_get_target(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_user_data(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(cand_panel);
        if(id == 0) {
            pinyin_page_proc(obj, 0);
            return;
        }
        if(id == (LV_IME_PINYIN_CAND_TEXT_NUM + 1)) {
            pinyin_page_proc(obj, 1);
            return;
        }

        const char * txt = lv_btnmatrix_get_btn_text(cand_panel, id);
        lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
        uint16_t index = 0;
        for(index = 0; index < pinyin_ime->ta_count; index++)
            lv_textarea_del_char(ta);

        lv_textarea_add_text(ta, txt);

        pinyin_ime_clear_data(obj);
    }
}


static void pinyin_input_proc(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->cand_str = pinyin_search_matching(obj, pinyin_ime->input_char, &pinyin_ime->cand_num);
    if(pinyin_ime->cand_str == NULL) {
        return;
    }

    pinyin_ime->py_page = 0;

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[i * 3 + j];
        }
    }

    lv_obj_clear_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

static void pinyin_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;
    uint16_t page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
    uint16_t sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

    if(dir == 0) {
        if(pinyin_ime->py_page) {
            pinyin_ime->py_page--;
        }
    }
    else {
        if(sur == 0) {
            page_num -= 1;
        }
        if(pinyin_ime->py_page < page_num) {
            pinyin_ime->py_page++;
        }
        else return;
    }

    for(uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++) {
        memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    uint16_t offset = pinyin_ime->py_page * (3 * LV_IME_PINYIN_CAND_TEXT_NUM);
    for(uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++) {
        if((sur > 0) && (pinyin_ime->py_page == page_num)) {
            if(i > sur)
                break;
        }
        for(uint8_t j = 0; j < 3; j++) {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[offset + (i * 3) + j];
        }
    }
}


static void lv_ime_pinyin_style_change_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    if(code == LV_EVENT_STYLE_CHANGED) {
        const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(pinyin_ime->cand_panel, font, 0);
    }
}


static void init_pinyin_dict(lv_obj_t * obj, lv_pinyin_dict_t * dict)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    char headletter = 'a';
    uint16_t offset_sum = 0;
    uint16_t offset_count = 0;
    uint16_t letter_calc = 0;

    pinyin_ime->dict = dict;

    for(uint16_t i = 0; ; i++) {
        if((NULL == (dict[i].py)) || (NULL == (dict[i].py_mb))) {
            headletter = dict[i - 1].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc] = offset_count;
            break;
        }

        if(headletter == (dict[i].py[0])) {
            offset_count++;
        }
        else {
            headletter = dict[i].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc - 1] = offset_count;
            offset_sum += offset_count;
            pinyin_ime->py_pos[letter_calc] = offset_sum;

            offset_count = 1;
        }
    }
}


static char * pinyin_search_matching(lv_obj_t * obj, char * py_str, uint16_t * cand_num)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ;
    uint8_t index, len = 0, offset;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return NULL;
    if(*py_str == 'i')     return NULL;
    if(*py_str == 'u')     return NULL;
    if(*py_str == 'v')     return NULL;

    offset = py_str[0] - 'a';
    len = strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            // The Chinese character in UTF-8 encoding format is 3 bytes
            * cand_num = strlen((const char *)(cpHZ->py_mb)) / 3;
            return (char *)(cpHZ->py_mb);
        }
        cpHZ++;
    }
    return NULL;
}

static void pinyin_ime_clear_data(lv_obj_t * obj)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;


#if LV_IME_PINYIN_USE_K9_MODE
    if(pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) {
        pinyin_ime->k9_input_str_len = 0;
        pinyin_ime->k9_py_ll_pos = 0;
        pinyin_ime->k9_legal_py_count = 0;
        lv_memset_00(pinyin_ime->k9_input_str,  LV_IME_PINYIN_K9_MAX_INPUT);
        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
    }
#endif

    pinyin_ime->ta_count = 0;
    lv_memset_00(lv_pinyin_cand_str, (sizeof(lv_pinyin_cand_str)));
    lv_memset_00(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));

    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}


#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t * obj)
{
    // lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for(btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++) {
        if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT"\0");
        }
        else if(py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1) {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else {
            strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        lv_btnm_def_pinyin_k9_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

    default_kb_ctrl_k9_map[0]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[4]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[5]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[9]  = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[10] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[14] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[15] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
    default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16] = LV_KEYBOARD_CTRL_BTN_FLAGS | 1;
}

static void pinyin_k9_get_legal_py(lv_obj_t * obj, char * k9_input, const char * py9_map[])
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = strlen(k9_input);

    if((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT)) {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    int count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while(index != -1) {
        if(index == len) {
            if(pinyin_k9_is_valid_py(obj, py_comp)) {
                if((count >= ll_len) || (ll_len == 0)) {
                    ll_index = _lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    strcpy(ll_index->py_str, py_comp);
                }
                else if((count < ll_len)) {
                    strcpy(ll_index->py_str, py_comp);
                    ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
            }
            index--;
        }
        else {
            flag = mark[index];
            if(flag < strlen(py9_map[k9_input[index] - '2'])) {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else {
                mark[index] = 0;
                index--;
            }
        }
    }

    if(count > 0) {
        pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}


/*true: visible; false: not visible*/
static bool pinyin_k9_is_valid_py(lv_obj_t * obj, char * py_str)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_pinyin_dict_t * cpHZ = NULL;
    uint8_t index = 0, len = 0, offset = 0;
    // uint16_t ret = 1;
    volatile uint8_t count = 0;

    if(*py_str == '\0')    return false;
    if(*py_str == 'i')     return false;
    if(*py_str == 'u')     return false;
    if(*py_str == 'v')     return false;

    offset = py_str[0] - 'a';
    len = strlen(py_str);

    cpHZ  = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while(count--) {
        for(index = 0; index < len; index++) {
            if(*(py_str + index) != *((cpHZ->py) + index)) {
                break;
            }
        }

        // perfect match
        if(len == 1 || index == len) {
            return true;
        }
        cpHZ++;
    }
    return false;
}


static void pinyin_k9_fill_cand(lv_obj_t * obj)
{
    static uint16_t len = 0;
    uint16_t index = 0, tmp_len = 0;
    ime_pinyin_k9_py_str_t * ll_index = NULL;

    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    tmp_len = pinyin_ime->k9_legal_py_count;

    if(tmp_len != len) {
        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        len = tmp_len;
    }

    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
    strcpy(pinyin_ime->input_char, ll_index->py_str);
    while(ll_index) {
        if((index >= LV_IME_PINYIN_K9_CAND_TEXT_NUM) || \
           (index >= pinyin_ime->k9_legal_py_count))
            break;

        strcpy(lv_pinyin_k9_cand_str[index], ll_index->py_str);
        ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
        index++;
    }
    pinyin_ime->k9_py_ll_pos = index;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    for(index = 0; index < pinyin_ime->k9_input_str_len; index++) {
        lv_textarea_del_char(ta);
    }
    pinyin_ime->k9_input_str_len = strlen(pinyin_ime->input_char);
    lv_textarea_add_text(ta, pinyin_ime->input_char);
}


static void pinyin_k9_cand_page_proc(lv_obj_t * obj, uint16_t dir)
{
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_obj_t * ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    uint16_t ll_len =  _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);

    if((ll_len > LV_IME_PINYIN_K9_CAND_TEXT_NUM) && (pinyin_ime->k9_legal_py_count > LV_IME_PINYIN_K9_CAND_TEXT_NUM)) {
        ime_pinyin_k9_py_str_t * ll_index = NULL;
        // uint16_t tmp_btn_str_len = 0;
        int count = 0;

        ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
        while(ll_index) {
            if(count >= pinyin_ime->k9_py_ll_pos)   break;

            ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
            count++;
        }

        if((NULL == ll_index) && (dir == 1))   return;

        lv_memset_00(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT"\0");
        strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");

        // next page
        if(dir == 1) {
            count = 0;
            while(ll_index) {
                if(count >= (LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1))
                    break;

                strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
                count++;
            }
            pinyin_ime->k9_py_ll_pos += count - 1;

        }
        // previous page
        else {
            count = LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1;
            ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index);
            while(ll_index) {
                if(count < 0)  break;

                strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the previous list*/
                count--;
            }

            if(pinyin_ime->k9_py_ll_pos > LV_IME_PINYIN_K9_CAND_TEXT_NUM)
                pinyin_ime->k9_py_ll_pos -= 1;
        }

        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

#endif  /*LV_IME_PINYIN_USE_K9_MODE*/

#endif  /*LV_USE_IME_PINYIN*/

