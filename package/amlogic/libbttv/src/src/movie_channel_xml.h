
#ifndef MOVIE_CHANNEL_XML_H
#define MOVIE_CHANNEL_XML_H

//#define MOVIE_HAS_HD

#ifdef MOVIE_HAS_HD
char *movie_channel_xml =\
"<?xml version=\"1.0\" encoding=\"GB2312\" ?>\
<movielist version=\"1.0\">\
<parent id=\"1\" title=\"影视排行榜\" page=\"1\" count=\"31\"/>\
<moviesets>\
\
<movie id=\"1\" ischannel=\"1\" size=\"50\">\
<name>高清影视人气榜</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://dy.n0808.com/rank_list/hd0.xml</child>\
</movie>\
\
<movie id=\"2\" ischannel=\"1\" size=\"50\">\
<name>高清影视热门榜</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://dy.n0808.com/rank_list/hd1.xml</child>\
</movie>\
\
<movie id=\"3\" ischannel=\"1\" size=\"50\">\
<name>高清影视最新榜</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://dy.n0808.com/rank_list/hd2.xml</child>\
</movie>\
\
<movie id=\"4\" ischannel=\"1\" size=\"50\">\
<name>最新大陆电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_neidi_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"5\" ischannel=\"1\" size=\"50\">\
<name>最新港台电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_gangtai_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"6\" ischannel=\"1\" size=\"50\">\
<name>最新欧美电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_oumei_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"7\" ischannel=\"1\" size=\"50\">\
<name>最新日韩电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_rihan_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"8\" ischannel=\"1\" size=\"50\">\
<name>最新大陆电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_neidi_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"9\" ischannel=\"1\" size=\"50\">\
<name>最新港台电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_gangtai_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"10\" ischannel=\"1\" size=\"50\">\
<name>最新欧美电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_oumei_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"11\" ischannel=\"1\" size=\"50\">\
<name>最新日韩电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_rihan_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"12\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-动作</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_action_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"13\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-冒险</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_adventure_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"14\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-喜剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_comedy_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"15\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-儿童</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_children_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"16\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-家庭</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_family_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"17\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-历史</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_history_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"18\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-恐怖</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_horror_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"19\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-爱情</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_romance_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"20\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-科幻</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_sci-fi_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"21\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-励志</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_encourage_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"22\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-武侠</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_wx_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"23\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-家庭</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jt_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"24\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-儿童</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_et_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"25\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-动画</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_dh_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"26\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-历史</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_ls_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"27\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-悬疑</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_xy_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"28\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-都市</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_ds_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"29\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-科幻</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_kh_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"30\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-军事</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jd_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"31\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-剧情</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jq_top50_gbk.xml</child>\
</movie>\
\
</moviesets>\
</movielist>\
";
#else
char *movie_channel_xml =\
"<?xml version=\"1.0\" encoding=\"GB2312\" ?>\
<movielist version=\"1.0\">\
<parent id=\"1\" title=\"影视排行榜\" page=\"1\" count=\"28\"/>\
<moviesets>\
\
<movie id=\"1\" ischannel=\"1\" size=\"50\">\
<name>最新大陆电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_neidi_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"2\" ischannel=\"1\" size=\"50\">\
<name>最新港台电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_gangtai_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"3\" ischannel=\"1\" size=\"50\">\
<name>最新欧美电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_oumei_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"4\" ischannel=\"1\" size=\"50\">\
<name>最新日韩电影</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_rihan_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"5\" ischannel=\"1\" size=\"50\">\
<name>最新大陆电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_neidi_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"6\" ischannel=\"1\" size=\"50\">\
<name>最新港台电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_gangtai_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"7\" ischannel=\"1\" size=\"50\">\
<name>最新欧美电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_oumei_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"8\" ischannel=\"1\" size=\"50\">\
<name>最新日韩电视剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_rihan_new_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"9\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-动作</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_action_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"10\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-冒险</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_adventure_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"11\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-喜剧</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_comedy_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"12\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-儿童</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_children_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"13\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-家庭</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_family_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"14\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-历史</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_history_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"15\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-恐怖</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_horror_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"16\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-爱情</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_romance_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"17\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-科幻</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_sci-fi_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"18\" ischannel=\"1\" size=\"50\">\
<name>电影分类排行-励志</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/movie_genre_encourage_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"19\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-武侠</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_wx_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"20\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-家庭</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jt_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"21\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-儿童</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_et_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"22\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-动画</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_dh_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"23\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-历史</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_ls_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"24\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-悬疑</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_xy_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"25\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-都市</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_ds_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"26\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-科幻</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_kh_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"27\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-军事</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jd_top50_gbk.xml</child>\
</movie>\
\
<movie id=\"28\" ischannel=\"1\" size=\"50\">\
<name>电视剧分类排行-剧情</name>\
<movieinfo></movieinfo>\
<desc></desc>\
<pic></pic>\
<child>http://data.movie.xunlei.com/xiazai/teleplay_genre_jq_top50_gbk.xml</child>\
</movie>\
\
</moviesets>\
</movielist>\
";
#endif //MOVIE_HAS_HD

#endif // MOVIE_CHANNEL_XML_H

