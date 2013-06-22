<!DOCTYPE html>
<!-- Server: sfn-web-10 -->


  










<!--[if lt IE 7 ]> <html lang="en" class="no-js ie6"> <![endif]-->
<!--[if IE 7 ]>    <html lang="en" class="no-js ie7"> <![endif]-->
<!--[if IE 8 ]>    <html lang="en" class="no-js ie8"> <![endif]-->
<!--[if IE 9 ]>    <html lang="en" class="no-js ie9"> <![endif]-->
<!--[if (gt IE 9)|!(IE)]>--> <html lang="en" class="no-js"> <!--<![endif]-->
  <head>
    <meta content="text/html; charset=UTF-8" http-equiv="content-type"/>
    <title>
  Money Manager Ex / Code /
  [r4134]
  /branches/mmex_dbview_wx29/mmex/sql/tables_v1.sql
</title>
    
<meta id="webtracker" name="webtracker" content='{&#34;event_id&#34;: &#34;2e7dcd62-db19-11e2-9dc0-0200ac1d293c&#34;, &#34;project&#34;: &#34;moneymanagerex&#34;, &#34;action_type&#34;: &#34;svn&#34;}' />

<script src="http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/js/sftheme/modernizr.custom.90514.js"></script>

<script src="http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/js/sftheme/jquery-1.8.0.min.js"></script>

<script src="http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/js/sftheme/header.js"></script>
<!--[if lt IE 7 ]>
  <script src="http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/js/sftheme/dd_belatedpng.js"></script>
  <script> DD_belatedPNG.fix('img, .png_bg'); //fix any <img> or .png_bg background-images </script>
<![endif]-->
<link href='//fonts.googleapis.com/css?family=Ubuntu:regular' rel='stylesheet' type='text/css'>
<style type="text/css">
    @font-face {
        font-family: "Pictos";
        src: url('http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/css/fonts/sftheme/pictos-web.eot');
        src: local("☺"), url('http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/css/fonts/sftheme/pictos-web.woff') format('woff'), url('http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/css/fonts/sftheme/pictos-web.ttf') format('truetype'), url('http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/css/fonts/sftheme/pictos-web.svg') format('svg');
    }
</style>
    <script type="text/javascript">
            /*jslint onevar: false, nomen: false, evil: true, css: true, plusplus: false, white: false, forin: true, on: true, immed: false */
            /*global confirm, alert, unescape, window, jQuery, $, net, COMSCORE */
    </script>
    
      <!-- ew:head_css -->

    
      <link rel="stylesheet"
                type="text/css"
                href="http://a.fsdn.com/allura/nf/1371835376/_ew_/_slim/css?href=allura%2Fcss%2Fforge%2Fhilite.css"
                >
    
      <link rel="stylesheet"
                type="text/css"
                href="http://a.fsdn.com/allura/nf/1371835376/_ew_/theme/sftheme/css/forge.css"
                >
    
      
<!-- /ew:head_css -->

    
    
    
      <!-- ew:head_js -->

    
      
<!-- /ew:head_js -->

    
    

    
      <style type="text/css">
        #page-body.project---init-- #top_nav { display: none; }
#page-body.project---init-- #nav_menu_holder { display: none; margin-bottom: 0; }
#page-body.project---init-- #content_base {margin-top: 0; }
      </style>
    
    
    <link rel="alternate" type="application/rss+xml" title="RSS" href="/p/moneymanagerex/code/feed.rss"/>
    <link rel="alternate" type="application/atom+xml" title="Atom" href="/p/moneymanagerex/code/feed.atom"/>

      <style>.XwNLKqVxtDczfFitYcVI { display:none }</style>

    
    
    
    


<script type="text/javascript">
    var _gaq = _gaq || [];

    function _add_tracking(prefix, tracking_id, send_user) {
        _gaq.push([prefix+'._setAccount', tracking_id]);
        _gaq.push([prefix+'._setCustomVar', 1, 'Page Type', 'svn', 3]);_gaq.push([prefix+'._trackPageview']);
    }
      _add_tracking('sfnt1', 'UA-32013-6', true);
      _add_tracking('sfnt2', 'UA-36130941-1', true);
    

    (function() {
        var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
        ga.src = ('https:' === document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
        var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
    })();
</script>
  </head>

  <body id="forge">
    
      <!-- ew:body_top_js -->

    
      
<!-- /ew:body_top_js -->

    
    
<header id="site-header">
    <div class="wrapper">
        <a href="/" class="logo">
            <span>SourceForge</span>
        </a>
        
        <form method="get" action="/directory/">
            <input type="text" id="words" name="q" placeholder="Search">
        </form>
        
        <!--Switch to {language}-->
        <nav id="nav-site">
            <a href="/directory/" title="Browse our software.">Browse</a>
            <a href="/directory/enterprise" title="Browse our Enterprise software.">Enterprise</a>
            <a href="/blog/" title="Read the latest news from the SF HQ.">Blog</a>
            <a href="/support" title="Contact us for help and feedback.">Help</a>
            <a href="/jobs?source=header" title="Search 80k+ tech jobs." class="featured-link">Jobs</a>
        </nav>
        <nav id="nav-account">
            
              <div class="logged_out">
                <a href="/account/login.php">Log In</a>
                <span>or</span>
                <a href="https://sourceforge.net/user/registration/">Join</a>
              </div>
            
        </nav>
        
    </div>
</header>
<header id="site-sec-header">
    <div class="wrapper">
        <nav id="nav-hubs">
            <h4>Solution Centers</h4>
            <a href="http://ibmsmartercommerce.sourceforge.net/">Smarter Commerce</a>
            <a href="http://goparallel.sourceforge.net/">Go Parallel</a>
            <a href="http://html5center.sourceforge.net/">HTML5</a>
            <a href="http://ibmsmarteritservices.sourceforge.net/">Smarter IT</a>
        </nav>
        <nav id="nav-collateral">
            <a href="http://library.geeknetmedia.com">Resources</a>
            
            <a href="">Newsletters</a>
            
        </nav>
    </div>
</header>
    
    <section id="page-body" class=" neighborhood-Projects project-moneymanagerex mountpoint-code">
	  <div class="grid-24" id="nav_menu_holder">
            
            



  
    <nav id="breadcrumbs">
        <ul>
            <li><a href="/">Home</a></li>
            <li><a href="/directory">Browse</a></li>
            
            
            
                
            
            
                
                    <li><a href="/directory/office/">Office/Business</a></li>
                
            
                
                    <li><a href="/directory/office/financial/">Financial</a></li>
                
            
                
                    <li><a href="/directory/office/financial/personalfinance/">Personal finance</a></li>
                
            
                
                    <li>Code</li>
                
            
        </ul>
    </nav>
  
    
      <img src="/p/moneymanagerex//icon?2013-04-02 20:06:01+00:00" class="project_icon" alt="Project Logo">
    
    <h1 class="project_title">
      <a href="/p/moneymanagerex/" class="project_link">Money Manager Ex</a>
    </h1>
    
    
    
    <h2 class="project_summary">
        
    </h2>
    <div class="brought-by with-icon">
        Brought to you by:
        
        
            
                <a href="/u/guanlisheng/">guanlisheng</a>,
            
            
                <a href="/u/madhan/">madhan</a>,
            
            
                <a href="/u/stef145g/">stef145g</a>,
            
            
                <a href="/u/vomikan/">vomikan</a>
            </div>

            
      </div>
      <div id="top_nav" class="">
        
        
<ul class="dropdown">
  
    <li class="">
        <a href="/projects/moneymanagerex/" class="ui-icon-tool-summary">
            Summary
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/projects/moneymanagerex/files/" class="ui-icon-tool-files">
            Files
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/projects/moneymanagerex/reviews" class="ui-icon-tool-reviews">
            Reviews
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/projects/moneymanagerex/support" class="ui-icon-tool-support">
            Support
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/p/moneymanagerex/wiki/" class="ui-icon-tool-wiki">
            Wiki
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/p/moneymanagerex/mailman/" class="ui-icon-tool-mailman">
            Mailing Lists
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/p/moneymanagerex/_list/tickets" class="ui-icon-tool-tickets">
            Tickets ▾
        </a>
        
        
            <ul>
                
                    <li class=""><a href="/p/moneymanagerex/patches/">Patches</a></li>
                
                    <li class=""><a href="/p/moneymanagerex/bugs/">Bugs</a></li>
                
                    <li class=""><a href="/p/moneymanagerex/feature-requests/">Feature Requests</a></li>
                
            </ul>
        
    </li>
	
    <li class="">
        <a href="/p/moneymanagerex/news/" class="ui-icon-tool-blog">
            News
        </a>
        
        
    </li>
	
    <li class="">
        <a href="/p/moneymanagerex/donate/" class="ui-icon-tool-link">
            Donate
        </a>
        
        
    </li>
	
    <li class="selected">
        <a href="/p/moneymanagerex/code/" class="ui-icon-tool-svn">
            Code
        </a>
        
        
    </li>
	
</ul>

        
      </div>
      <div id="content_base">
			  
			    
          


<div id="sidebar">
  
    <div>&nbsp;</div>
  
    
    
      
        
    
      <ul class="sidebarmenu">
      
    
    <li>
      <a href="/p/moneymanagerex/code/commit_browser"><b data-icon="o" class="ico ico-folder"></b> <span>Browse Commits</span></a>
    </li>
  
      
    
    
      </ul>
      
    
    
</div>
          
          
			  
			  
          
        
        <div class="grid-20 pad">
          <h2 class="dark title">
<a href="/p/moneymanagerex/code/4134/">[r4134]</a>:

  
  
    <a href="./../../../">branches</a> /
    
  
    <a href="./../../">mmex_dbview_wx29</a> /
    
  
    <a href="./../">mmex</a> /
    
  
    <a href="./">sql</a> /
    
  
 tables_v1.sql

            <!-- actions -->
            <small>
            
<a href="/p/moneymanagerex/code/4134/log/?path=/branches/mmex_dbview_wx29/mmex/sql/tables_v1.sql">
  <b data-icon="N" class="ico ico-history" title="History"> </b> History
</a>

            </small>
            <!-- /actions -->
          </h2>
		
          <div>
            
  

            
  

  

  
    <p><a href="?format=raw">Download this file</a></p>
    <div class="clip grid-19 codebrowser">
      <h3>
        <span class="ico-l"><b data-icon="n" class="ico ico-table"></b> tables_v1.sql</span>
        &nbsp;&nbsp;
        175 lines (152 with data), 3.5 kB
      </h3>
      
        <table class="codehilitetable"><tr><td class="linenos"><div class="linenodiv"><pre>  1
  2
  3
  4
  5
  6
  7
  8
  9
 10
 11
 12
 13
 14
 15
 16
 17
 18
 19
 20
 21
 22
 23
 24
 25
 26
 27
 28
 29
 30
 31
 32
 33
 34
 35
 36
 37
 38
 39
 40
 41
 42
 43
 44
 45
 46
 47
 48
 49
 50
 51
 52
 53
 54
 55
 56
 57
 58
 59
 60
 61
 62
 63
 64
 65
 66
 67
 68
 69
 70
 71
 72
 73
 74
 75
 76
 77
 78
 79
 80
 81
 82
 83
 84
 85
 86
 87
 88
 89
 90
 91
 92
 93
 94
 95
 96
 97
 98
 99
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
131
132
133
134
135
136
137
138
139
140
141
142
143
144
145
146
147
148
149
150
151
152
153
154
155
156
157
158
159
160
161
162
163
164
165
166
167
168
169
170
171
172
173
174</pre></div></td><td class="code"><div class="codehilite"><pre><div id="l1" class="code_block"><span class="c1">-- Describe ACCOUNTLIST_V1</span>
</div><div id="l2" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">ACCOUNTLIST_V1</span><span class="p">(</span>
</div><div id="l3" class="code_block"><span class="n">ACCOUNTID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l4" class="code_block"><span class="p">,</span> <span class="n">ACCOUNTNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l5" class="code_block"><span class="p">,</span> <span class="n">ACCOUNTTYPE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> 
</div><div id="l6" class="code_block"><span class="p">,</span> <span class="n">ACCOUNTNUM</span> <span class="nb">TEXT</span>
</div><div id="l7" class="code_block"><span class="p">,</span> <span class="n">STATUS</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l8" class="code_block"><span class="p">,</span> <span class="n">NOTES</span> <span class="nb">TEXT</span> 
</div><div id="l9" class="code_block"><span class="p">,</span> <span class="n">HELDAT</span> <span class="nb">TEXT</span> 
</div><div id="l10" class="code_block"><span class="p">,</span> <span class="n">WEBSITE</span> <span class="nb">TEXT</span> 
</div><div id="l11" class="code_block"><span class="p">,</span> <span class="n">CONTACTINFO</span> <span class="nb">TEXT</span>
</div><div id="l12" class="code_block"><span class="p">,</span> <span class="n">ACCESSINFO</span> <span class="nb">TEXT</span> 
</div><div id="l13" class="code_block"><span class="p">,</span> <span class="n">INITIALBAL</span> <span class="nb">numeric</span> 
</div><div id="l14" class="code_block"><span class="p">,</span> <span class="n">FAVORITEACCT</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l15" class="code_block"><span class="p">,</span> <span class="n">CURRENCYID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l16" class="code_block"><span class="p">);</span>
</div><div id="l17" class="code_block">
</div><div id="l18" class="code_block"><span class="c1">-- Describe ASSETS_V1</span>
</div><div id="l19" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">ASSETS_V1</span><span class="p">(</span>
</div><div id="l20" class="code_block"><span class="n">ASSETID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l21" class="code_block"><span class="p">,</span> <span class="n">STARTDATE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> 
</div><div id="l22" class="code_block"><span class="p">,</span> <span class="n">ASSETNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l23" class="code_block"><span class="p">,</span> <span class="n">VALUE</span> <span class="nb">numeric</span>
</div><div id="l24" class="code_block"><span class="p">,</span> <span class="n">VALUECHANGE</span> <span class="nb">TEXT</span>
</div><div id="l25" class="code_block"><span class="p">,</span> <span class="n">NOTES</span> <span class="nb">TEXT</span>
</div><div id="l26" class="code_block"><span class="p">,</span> <span class="n">VALUECHANGERATE</span> <span class="nb">numeric</span>
</div><div id="l27" class="code_block"><span class="p">,</span> <span class="n">ASSETTYPE</span> <span class="nb">TEXT</span>
</div><div id="l28" class="code_block"><span class="p">);</span>
</div><div id="l29" class="code_block">
</div><div id="l30" class="code_block">
</div><div id="l31" class="code_block"><span class="c1">-- Describe BILLSDEPOSITS_V1</span>
</div><div id="l32" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">BILLSDEPOSITS_V1</span><span class="p">(</span>
</div><div id="l33" class="code_block"><span class="n">BDID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l34" class="code_block"><span class="p">,</span> <span class="n">ACCOUNTID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l35" class="code_block"><span class="p">,</span> <span class="n">TOACCOUNTID</span> <span class="nb">integer</span>
</div><div id="l36" class="code_block"><span class="p">,</span> <span class="n">PAYEEID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l37" class="code_block"><span class="p">,</span> <span class="n">TRANSCODE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l38" class="code_block"><span class="p">,</span> <span class="n">TRANSAMOUNT</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l39" class="code_block"><span class="p">,</span> <span class="n">STATUS</span> <span class="nb">TEXT</span>
</div><div id="l40" class="code_block"><span class="p">,</span> <span class="n">TRANSACTIONNUMBER</span> <span class="nb">TEXT</span>
</div><div id="l41" class="code_block"><span class="p">,</span> <span class="n">NOTES</span> <span class="nb">TEXT</span>
</div><div id="l42" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l43" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l44" class="code_block"><span class="p">,</span> <span class="n">TRANSDATE</span> <span class="nb">TEXT</span>
</div><div id="l45" class="code_block"><span class="p">,</span> <span class="n">FOLLOWUPID</span> <span class="nb">integer</span>
</div><div id="l46" class="code_block"><span class="p">,</span> <span class="n">TOTRANSAMOUNT</span> <span class="nb">numeric</span>
</div><div id="l47" class="code_block"><span class="p">,</span> <span class="n">REPEATS</span> <span class="nb">integer</span>
</div><div id="l48" class="code_block"><span class="p">,</span> <span class="n">NEXTOCCURRENCEDATE</span> <span class="nb">TEXT</span>
</div><div id="l49" class="code_block"><span class="p">,</span> <span class="n">NUMOCCURRENCES</span> <span class="nb">numeric</span>
</div><div id="l50" class="code_block"><span class="p">);</span>
</div><div id="l51" class="code_block">
</div><div id="l52" class="code_block"><span class="c1">-- Describe BUDGETSPLITTRANSACTIONS_V1</span>
</div><div id="l53" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">BUDGETSPLITTRANSACTIONS_V1</span><span class="p">(</span>
</div><div id="l54" class="code_block"><span class="n">SPLITTRANSID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l55" class="code_block"><span class="p">,</span> <span class="n">TRANSID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l56" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l57" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l58" class="code_block"><span class="p">,</span> <span class="n">SPLITTRANSAMOUNT</span> <span class="nb">numeric</span>
</div><div id="l59" class="code_block"><span class="p">);</span>
</div><div id="l60" class="code_block">
</div><div id="l61" class="code_block"><span class="c1">-- Describe BUDGETTABLE_V1</span>
</div><div id="l62" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">BUDGETTABLE_V1</span><span class="p">(</span>
</div><div id="l63" class="code_block"><span class="n">BUDGETENTRYID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l64" class="code_block"><span class="p">,</span> <span class="n">BUDGETYEARID</span> <span class="nb">integer</span>
</div><div id="l65" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l66" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l67" class="code_block"><span class="p">,</span> <span class="n">PERIOD</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l68" class="code_block"><span class="p">,</span> <span class="n">AMOUNT</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l69" class="code_block"><span class="p">);</span>
</div><div id="l70" class="code_block">
</div><div id="l71" class="code_block"><span class="c1">-- Describe BUDGETYEAR_V1</span>
</div><div id="l72" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">BUDGETYEAR_V1</span><span class="p">(</span>
</div><div id="l73" class="code_block"><span class="n">BUDGETYEARID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l74" class="code_block"><span class="p">,</span> <span class="n">BUDGETYEARNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l75" class="code_block"><span class="p">);</span>
</div><div id="l76" class="code_block">
</div><div id="l77" class="code_block"><span class="c1">-- Describe CATEGORY_V1</span>
</div><div id="l78" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">CATEGORY_V1</span><span class="p">(</span>
</div><div id="l79" class="code_block"><span class="n">CATEGID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l80" class="code_block"><span class="p">,</span> <span class="n">CATEGNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l81" class="code_block"><span class="p">);</span>
</div><div id="l82" class="code_block">
</div><div id="l83" class="code_block">
</div><div id="l84" class="code_block"><span class="c1">-- Describe CHECKINGACCOUNT_V1</span>
</div><div id="l85" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">CHECKINGACCOUNT_V1</span><span class="p">(</span>
</div><div id="l86" class="code_block"><span class="n">TRANSID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l87" class="code_block"><span class="p">,</span> <span class="n">ACCOUNTID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l88" class="code_block"><span class="p">,</span> <span class="n">TOACCOUNTID</span> <span class="nb">integer</span>
</div><div id="l89" class="code_block"><span class="p">,</span> <span class="n">PAYEEID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l90" class="code_block"><span class="p">,</span> <span class="n">TRANSCODE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l91" class="code_block"><span class="p">,</span> <span class="n">TRANSAMOUNT</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l92" class="code_block"><span class="p">,</span> <span class="n">STATUS</span> <span class="nb">TEXT</span>
</div><div id="l93" class="code_block"><span class="p">,</span> <span class="n">TRANSACTIONNUMBER</span> <span class="nb">TEXT</span>
</div><div id="l94" class="code_block"><span class="p">,</span> <span class="n">NOTES</span> <span class="nb">TEXT</span>
</div><div id="l95" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l96" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l97" class="code_block"><span class="p">,</span> <span class="n">TRANSDATE</span> <span class="nb">TEXT</span>
</div><div id="l98" class="code_block"><span class="p">,</span> <span class="n">FOLLOWUPID</span> <span class="nb">integer</span>
</div><div id="l99" class="code_block"><span class="p">,</span> <span class="n">TOTRANSAMOUNT</span> <span class="nb">numeric</span>
</div><div id="l100" class="code_block"><span class="p">);</span>
</div><div id="l101" class="code_block">
</div><div id="l102" class="code_block">
</div><div id="l103" class="code_block"><span class="c1">-- Describe CURRENCYFORMATS_V1</span>
</div><div id="l104" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">CURRENCYFORMATS_V1</span><span class="p">(</span>
</div><div id="l105" class="code_block"><span class="n">CURRENCYID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l106" class="code_block"><span class="p">,</span> <span class="n">CURRENCYNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l107" class="code_block"><span class="p">,</span> <span class="n">PFX_SYMBOL</span> <span class="nb">TEXT</span>
</div><div id="l108" class="code_block"><span class="p">,</span> <span class="n">SFX_SYMBOL</span> <span class="nb">TEXT</span>
</div><div id="l109" class="code_block"><span class="p">,</span> <span class="n">DECIMAL_POINT</span> <span class="nb">TEXT</span>
</div><div id="l110" class="code_block"><span class="p">,</span> <span class="n">GROUP_SEPARATOR</span> <span class="nb">TEXT</span>
</div><div id="l111" class="code_block"><span class="p">,</span> <span class="n">UNIT_NAME</span> <span class="nb">TEXT</span>
</div><div id="l112" class="code_block"><span class="p">,</span> <span class="n">CENT_NAME</span> <span class="nb">TEXT</span>
</div><div id="l113" class="code_block"><span class="p">,</span> <span class="k">SCALE</span> <span class="nb">integer</span>
</div><div id="l114" class="code_block"><span class="p">,</span> <span class="n">BASECONVRATE</span> <span class="nb">numeric</span>
</div><div id="l115" class="code_block"><span class="p">,</span> <span class="n">CURRENCY_SYMBOL</span> <span class="nb">TEXT</span>
</div><div id="l116" class="code_block"><span class="p">);</span>
</div><div id="l117" class="code_block">
</div><div id="l118" class="code_block">
</div><div id="l119" class="code_block"><span class="c1">-- Describe INFOTABLE_V1</span>
</div><div id="l120" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">INFOTABLE_V1</span><span class="p">(</span>
</div><div id="l121" class="code_block"><span class="n">INFOID</span> <span class="nb">integer</span> <span class="k">not</span> <span class="k">null</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l122" class="code_block"><span class="p">,</span> <span class="n">INFONAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l123" class="code_block"><span class="p">,</span> <span class="n">INFOVALUE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l124" class="code_block"><span class="p">);</span>
</div><div id="l125" class="code_block">
</div><div id="l126" class="code_block">
</div><div id="l127" class="code_block"><span class="c1">-- Describe PAYEE_V1</span>
</div><div id="l128" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">PAYEE_V1</span><span class="p">(</span>
</div><div id="l129" class="code_block"><span class="n">PAYEEID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l130" class="code_block"><span class="p">,</span> <span class="n">PAYEENAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l131" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l132" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l133" class="code_block"><span class="p">);</span>
</div><div id="l134" class="code_block">
</div><div id="l135" class="code_block">
</div><div id="l136" class="code_block"><span class="c1">-- Describe SPLITTRANSACTIONS_V1</span>
</div><div id="l137" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">SPLITTRANSACTIONS_V1</span><span class="p">(</span>
</div><div id="l138" class="code_block"><span class="n">SPLITTRANSID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l139" class="code_block"><span class="p">,</span> <span class="n">TRANSID</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l140" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span>
</div><div id="l141" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGID</span> <span class="nb">integer</span>
</div><div id="l142" class="code_block"><span class="p">,</span> <span class="n">SPLITTRANSAMOUNT</span> <span class="nb">numeric</span>
</div><div id="l143" class="code_block"><span class="p">);</span>
</div><div id="l144" class="code_block">
</div><div id="l145" class="code_block">
</div><div id="l146" class="code_block"><span class="c1">-- Describe STOCK_V1</span>
</div><div id="l147" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">STOCK_V1</span><span class="p">(</span>
</div><div id="l148" class="code_block"><span class="n">STOCKID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l149" class="code_block"><span class="p">,</span> <span class="n">HELDAT</span> <span class="nb">integer</span> 
</div><div id="l150" class="code_block"><span class="p">,</span> <span class="n">PURCHASEDATE</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l151" class="code_block"><span class="p">,</span> <span class="n">STOCKNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l152" class="code_block"><span class="p">,</span> <span class="n">SYMBOL</span> <span class="nb">TEXT</span>
</div><div id="l153" class="code_block"><span class="p">,</span> <span class="n">NUMSHARES</span> <span class="nb">numeric</span>
</div><div id="l154" class="code_block"><span class="p">,</span> <span class="n">PURCHASEPRICE</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l155" class="code_block"><span class="p">,</span> <span class="n">NOTES</span> <span class="nb">TEXT</span>
</div><div id="l156" class="code_block"><span class="p">,</span> <span class="n">CURRENTPRICE</span> <span class="nb">numeric</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l157" class="code_block"><span class="p">,</span> <span class="n">VALUE</span> <span class="nb">numeric</span>
</div><div id="l158" class="code_block"><span class="p">,</span> <span class="n">COMMISSION</span> <span class="nb">numeric</span>
</div><div id="l159" class="code_block"><span class="p">);</span>
</div><div id="l160" class="code_block">
</div><div id="l161" class="code_block">
</div><div id="l162" class="code_block"><span class="c1">-- Describe SUBCATEGORY_V1</span>
</div><div id="l163" class="code_block"><span class="k">CREATE</span> <span class="k">TABLE</span> <span class="n">SUBCATEGORY_V1</span><span class="p">(</span>
</div><div id="l164" class="code_block"><span class="n">SUBCATEGID</span> <span class="nb">integer</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l165" class="code_block"><span class="p">,</span> <span class="n">SUBCATEGNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l166" class="code_block"><span class="p">,</span> <span class="n">CATEGID</span> <span class="nb">integer</span> <span class="k">NOT</span> <span class="k">NULL</span>
</div><div id="l167" class="code_block"><span class="p">);</span>
</div><div id="l168" class="code_block">
</div><div id="l169" class="code_block"><span class="c1">-- Describe SETTING_V1</span>
</div><div id="l170" class="code_block"><span class="k">create</span> <span class="k">table</span> <span class="n">SETTING_V1</span><span class="p">(</span>
</div><div id="l171" class="code_block"><span class="n">SETTINGID</span> <span class="nb">integer</span> <span class="k">not</span> <span class="k">null</span> <span class="k">primary</span> <span class="k">key</span>
</div><div id="l172" class="code_block"><span class="p">,</span> <span class="n">SETTINGNAME</span> <span class="nb">TEXT</span> <span class="k">NOT</span> <span class="k">NULL</span> <span class="k">UNIQUE</span>
</div><div id="l173" class="code_block"><span class="p">,</span> <span class="n">SETTINGVALUE</span> <span class="nb">TEXT</span>
</div><div id="l174" class="code_block"><span class="p">);</span>
</div></pre></div>
</td></tr></table>
      
    </div>
  

          </div>
			
          
        </div>
      </div>
    </section>
      
<footer id="site-footer">
    <div class="wrapper">
        <nav>
            <h5>SourceForge</h5>
            <a href="/about">About</a>
            <a href="/blog/category/sitestatus/">Site Status</a>
            <a href="http://twitter.com/sfnet_ops">@sfnet_ops</a>
        </nav>
        <nav>
            <h5>Find and Develop Software</h5>
            <a href="/create/">Create a Project</a>
            <a href="/directory/">Software Directory</a>
            <a href="/top">Top Downloaded Projects</a>
        </nav>
        <nav>
            <h5>Community</h5>
            <a href="/blog/">Blog</a>
            <a href="http://twitter.com/sourceforge">@sourceforge</a>
            <a href="/jobs?source=footer">Job Board</a>
        </nav>
        <nav>
            <h5>Help</h5>
            <a href="http://p.sf.net/sourceforge/docs">Site Documentation</a>
            <a href="/support">Support Request</a>
            <a href="http://p.sf.net/sourceforge/irc">Real-Time Support</a>
        </nav>
    </div>
</footer>
<footer id="site-copyright-footer">
    <div class="wrapper">
        <div id="copyright">
            Copyright &copy; 2013 SourceForge. All Rights Reserved.<br />
            SourceForge is a <a href="http://www.diceholdingsinc.com/phoenix.zhtml?c=211152&amp;p=irol-landing">Dice Holdings, Inc.</a> company.
        </div>
        <nav>
            <a href="http://slashdotmedia.com/terms-of-use">Terms</a>
            <a href="http://slashdotmedia.com/privacy-statement/">Privacy</a>
            <a href="http://slashdotmedia.com/opt-out-choices">Cookies/Opt Out</a>
            <a href="http://slashdotmedia.com">Advertise</a>
            <a href="http://sourceforge.jp/">SourceForge.JP</a>
        </nav>
    </div>
</footer>
    <div id="messages">
        
    </div>
    
      <!-- ew:body_js -->

    
      <script type="text/javascript" src="http://a.fsdn.com/allura/nf/1371835376/_ew_/_slim/js?href=allura%2Fjs%2Fjquery-base.js%3Ballura%2Fjs%2Fjquery.notify.js%3Ballura%2Fjs%2Fsylvester.js%3Ballura%2Fjs%2Fpb.transformie.min.js%3Ballura%2Fjs%2Fallura-base.js"></script>
    
      
<!-- /ew:body_js -->

    
    
      <!-- ew:body_js_tail -->

    
      
<!-- /ew:body_js_tail -->

    
    

<script type="text/javascript" src="http://a.fsdn.com/allura/nf/1371835376/_static_/js/spin.min.js"></script>
<script type="text/javascript">(function() {
  $('#access_urls .btn').click(function(evt){
    evt.preventDefault();
    var parent = $(this).parents('.btn-bar');
    $(parent).find('input').val($(this).attr('data-url'));
    $(parent).find('span').text($(this).attr('title')+' access');
    $(this).parent().children('.btn').removeClass('active');
    $(this).addClass('active');
  });
  $('#access_urls .btn').first().click();

  
  var repo_status = document.getElementById('repo_status');
  // The repo_status div will only be present if repo.status != 'ready'
  if (repo_status) {
    var opts = {
        lines: 9, // The number of lines to draw
        length: 4, // The length of each line
        width: 2, // The line thickness
        radius: 3, // The radius of the inner circle
        rotate: 0, // The rotation offset
        color: '#555', // #rgb or #rrggbb
        speed: 1, // Rounds per second
        trail: 60, // Afterglow percentage
        shadow: false, // Whether to render a shadow
        hwaccel: false, // Whether to use hardware acceleration
        className: 'spinner', // The CSS class to assign to the spinner
        zIndex: 2e9, // The z-index (defaults to 2000000000)
        top: 10, // Top position relative to parent in px
        left: 10 // Left position relative to parent in px
    };
    var spinner = new Spinner(opts).spin(repo_status);
    function check_status() {
        $.get('/p/moneymanagerex/code/status', function(data) {
            if (data.status === 'ready') {
                window.clearInterval(status_checker);
                spinner.opts.speed = 0;
                spinner.opts.opacity = 1;
                spinner.spin(repo_status)
                $('#repo_status h2').html('Repo status: ready. <a href=".">Click here to refresh this page.</a>');
            }
            else {
                $('#repo_status h2 span').html(data.status);
            }
        });
    }
    // Check repo status every 15 seconds
    var status_checker = window.setInterval(check_status, 15000);
    
  }
}());
</script>

<script type="text/javascript">(function() {
  $(window).bind('hashchange', function(e) {
    var hash = window.location.hash.substring(1);
	if ('originalEvent' in e && 'oldURL' in e.originalEvent) {
      $('#' + e.originalEvent.oldURL.split('#')[1]).css('background-color', 'transparent');
	}
    if (hash !== '' && hash.substring(0, 1) === 'l' && !isNaN(hash.substring(1))) {
      $('#' + hash).css('background-color', '#ffff99');
    }
  }).trigger('hashchange');

  $('.code_block').each(function(index, element) {
    $(element).bind('click', function() {
      var hash = window.location.hash.substring(1);
      if (hash !== '' && hash.substring(0, 1) === 'l' && !isNaN(hash.substring(1))) {
        $('#' + hash).css('background-color', 'transparent');
      }
      $(element).css('background-color', '#ffff99');
      window.location.href = '#' + $(element).attr('id');
    });
  });
}());
</script>

    
      
    
    
   
    <script src="//s.fsdn.com/con/js/webtracker.js" type="text/javascript"></script>
    <!-- Google Code for Remarketing tag -->
    <!-- Remarketing tags may not be associated with personally identifiable information or placed on pages related to sensitive categories. For instructions on adding this tag and more information on the above requirements, read the setup guide: google.com/ads/remarketingsetup -->
    <script type="text/javascript">
        /* <![CDATA[ */
        var google_conversion_id = 1002083962;
        var google_conversion_label = "G_uGCOaBlAQQ-qzq3QM";
        var google_custom_params = window.google_tag_params;
        var google_remarketing_only = true;
        /* ]]> */
    </script>
    <script type="text/javascript" src="//www.googleadservices.com/pagead/conversion.js"> </script>
    <noscript>
      <div style="display:inline;">
        <img height="1" width="1" style="border-style:none;" alt="" src="//googleads.g.doubleclick.net/pagead/viewthroughconversion/1002083962/?value=0&amp;label=G_uGCOaBlAQQ-qzq3QM&amp;guid=ON&amp;script=0"/>
      </div>
    </noscript>
    
  </body>
</html>