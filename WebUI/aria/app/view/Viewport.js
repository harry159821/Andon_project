Ext.define('AndonPortal.view.Viewport', {
    extend: 'Ext.container.Viewport',
    requires: [
        'Ext.Img',
        'Ext.layout.container.Border',
        'Ext.layout.container.Accordion',
        'Ext.tab.Panel',
        'Ext.form.Panel',
        'Ext.data.*',
        'Ext.grid.*',
        'Ext.tree.*',
        'Ext.tip.*',
        'Ext.ux.CheckColumn',
//        'Ext.ux.SqlSocket',
//        'AndonPortal.controller.Main',
        'Ext.ux.ajax.SimManager'
    ],
//    stroes:['Tree'],
    layout: 'border',
    initComponent: function() {
        var me = this,
            num = 1;
//        me.qws = Ext.create ('Ext.ux.SqlSocket',{
//                                 //url: "ws://127.0.0.1:12346",
//                                 id:'mysqlsocket',
//                                 url:"ws://10.208.110.75:12346",
//                                 listeners: {
//                                     ready: function (ws) {
//                                         //debugger;
//                                         AndonPortal.app.fireEvent('appSqlSocketReady',ws);
//                                         console.log('SqlSocket opened');
//                                         this.sql("TREE_GET_JSONTREE",
//                                                  "PLANT",
//                                                  "JSON_BRANCH",
//                                                          function(resp){
//                                                              var JSONdata=resp.map(function(node) {
//                                                                               return eval('['+node.JSON_BRANCH+']')[0];
//                                                                               });
//                                                              Ext.ux.ajax.SimManager.init({
//                                                                                              delay: 300,
//                                                                                              defaultSimlet: null
//                                                                                          }).register({'/json/treePanel': {
//                                                                                                              data: JSONdata,
//                                                                                                              stype: 'json'
//                                                                                                          }
//                                                                                                      });
//                                                              Ext.getStore('treeStore').load();
//                                                          });
//                                     }
//                                 }
//                             });

        me.items = [
//            {
//                xtype: 'sqlsocket',
//                        id:'mysqlsocket',
//                        url:"ws://10.208.110.75:12346",
//                        controller:'Main',
//                        listeners: {
//                            open:'onSqlSocketOpen'
//                        }
//            },
            {
            xtype: 'container',
            region: 'north',
            padding: '6 12',
            height: 40,
            
            ariaRole: 'banner',
            
            layout: {
                align: 'stretch',
                type: 'hbox'
            },
            
            items: [{
                xtype: 'image',
                alt: 'Sencha logo',
                src: 'resources/images/sencha-logo.png',
                width: 19,
                height: 28,
                style: {
                    'margin-right': '10px'
                }
            }, {
                xtype: 'component',
                ariaRole: 'heading',
                html: 'Ext JS Accessibility Demo',
                ariaAttributes: {
                    // Some screen readers will announce document contents upfront.
                    // In that case the region header component's html may be read
                    // twice: first as north container's aria-labelledby element,
                    // and then by itself as a div with a text node.
                    // The attribute below will instruct screen readers to skip
                    // the div text; it will be announced only as a region heading.
                    'aria-hidden': true
                }
            }, {
                xtype: 'component',
                flex: 3
            }]
        }, {
            xtype: 'panel',
            region: 'west',
            
            title: 'Accordion panels',
            collapsible: true,
            split: true,
            minWidth: 320,
            
            layout: {
                type: 'accordion',
                animate: true
            },
            
            tools: [{
                type: 'pin',
                tooltip: 'Pin the panel'
            }, {
                type: 'help',
                tooltip: 'Get panel help'
            }],
            
            items: [{
                xtype: 'mysimpletree',
                closable: false
            }]
        }, {
            xtype: 'tabpanel',
            title: 'Center tab panel',
            region: 'center',
            ariaRole: 'main',
            header: false,
            split: true,
            layout: 'fit',
            bodyStyle: 'background:white',
            defaults: {
                padding: 12,
                bodyStyle: 'background:white'
            },
            
            items: [{
                xtype: 'mysimplegrid'
            }]
        }];


        me.callParent();
    }
});
