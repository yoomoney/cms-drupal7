<?php
/**
 * @return array
 */
function yoomoney_api_admin_settings()
{
    global $base_url;
    $path         = drupal_get_path('module', 'yoomoney_api').'/'.'yoomoney_api.info';
    $info         = drupal_parse_info_file($path);
    $shopPassword = variable_get('yoomoney_api_secret', '');
    $prefix       = substr($shopPassword, 0, 4);

    if ($prefix == "test") {
        drupal_set_message(
            'Вы включили тестовый режим приема платежей. Проверьте, как проходит оплата. <a href="https://yookassa.ru/">Подробнее</a>',
            'warning'
        );
    }

    yoomoney_api_admin_remove_duplicate_messages('warning');

    $form                                      = array();
    $form['yoomoney_api_all']                   = array(
        '#type'        => 'fieldset',
        '#description' => t(
            'By operating the module, you automatically agree with <a href=\'https://yoomoney.ru/doc.xml?id=527132\' target=\'_blank\'>its Terms and Conditions of Use</a>.'
        ),
        '#weight'      => 10,
    );
    $form['yoomoney_api_all']['lbl_version']    = array(
        '#markup' => t('Version: ').$info['version'],
    );
    $form['yoomoney_api_all']['yoomoney_api_ip'] = array(
        '#type'          => 'hidden',
        '#title'         => t('Allowed IPs for callbacks'),
        '#default_value' => variable_get('yoomoney_api_ip', '0.0.0.0'),
        '#description'   => t(
            'The list of IP addresses which has access to payment callbacks. One per line.<br/>0.0.0.0 means allow from all.'
        ),
    );

    $form['vertical_tabs']                        = array(
        '#type'   => 'vertical_tabs',
        '#weight' => 20,
    );
    $form['yoomoney_api_kassa_tab']                = array(
        '#type'   => 'fieldset',
        '#title'  => t('YooKassa'),
        '#group'  => 'vertical_tabs',
        '#weight' => 10,
    );
    $form['yoomoney_api_kassa_tab']['lbl_setting'] = array(
        '#markup' => t('<h3>Store settings</h3>'),
    );
    $form['yoomoney_api_kassa_tab']['intro']       = array(
        '#markup' => t(
            'To start operating the module, you need to connect your store to <a target="_blank" href="https://yookassa.ru/en/">YooKassa</a>'
        ),
    );

    $form['yoomoney_api_kassa_tab']['yoomoney_api_shop'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Enable payment acceptance via YooKassa'),
        '#default_value' => variable_get('yoomoney_api_shop', 0),
    );


    $form['yoomoney_api_kassa_tab']['yoomoney_api_shop_id'] = array(
        '#type'          => 'textfield',
        '#title'         => t('shopId'),
        '#description'   => t('Copy shopId from your YooKassa\'s Merchant Profile'),
        '#default_value' => variable_get('yoomoney_api_shop_id', ''),
        '#size'          => 4,
    );
    $form['yoomoney_api_kassa_tab']['yoomoney_api_secret']  = array(
        '#type'          => 'textfield',
        '#title'         => t('Secret key'),
        '#description'   => t(
            'Issue and activate a secret key under your YooKassa\'s Merchant Profile. Then copy it here.'
        ),
        '#default_value' => variable_get('yoomoney_api_secret', ''),
        '#size'          => 60,
    );

    $paymode_states                                       = array(
        'invisible' => array(
            ':input[name="yoomoney_api_paymode"]' => array('value' => 'kassa'),
        ),
    );
    $form['yoomoney_api_kassa_tab']['yoomoney_api_paymode'] = array(
        '#type'          => 'select',
        '#title'         => t('Select payment method'),
        '#options'       => array(
            'kassa' => t('On YooKassa\'s site'),
            'shop'  => t('On the store\'s site'),
        ),
        '#default_value' => variable_get('yoomoney_api_paymode', 'kassa'),
    );
    /*$form['yoomoney_api_kassa_tab']['yoomoney_api_paylogo'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use logo of payment button'),
        '#default_value' => variable_get('yoomoney_api_paylogo', 1),
        '#states' => array(
            'invisible' => array(
                ':input[name="yoomoney_api_paymode"]' => array('value' => 'shop'),
            ),
        ),
    );*/
    $form['yoomoney_api_kassa_tab']['yoomoney_api_payment_method'] = array(
        '#type'          => 'checkboxes',
        '#title'         => t('Enabled payment methods'),
        '#options'       => yoomoney_api_get_payment_methods(),
        '#default_value' => variable_get('yoomoney_api_payment_method', array_keys(yoomoney_api_get_payment_methods())),
        '#states'        => $paymode_states,
    );

    $form['yoomoney_api_kassa_tab']['yoomoney_api_default_payment_method'] = array(
        '#type'          => 'radios',
        '#title'         => t('Default payment method'),
        '#options'       => yoomoney_api_get_payment_methods(),
        '#default_value' => variable_get('yoomoney_api_default_payment_method', YOOMONEY_API_DEFAULT_PAYMENT_METHOD),
        '#states'        => $paymode_states,
    );

    /*********** Transaction data ***********/
    if (module_exists('commerce')) {
        $form['yoomoney_api_kassa_tab']['yoomoney_api_description_template'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Transaction data'),
            '#description'   => t("Full description of the transaction that the user will see during the checkout process. You can find it in your YooKassa Merchant Profile. For example, \"Payment for order No. 72 by user@yoomoney.ru\". Limitations: no more than 128 symbols."),
            '#default_value' => variable_get('yoomoney_api_description_template', t('Payment for order No. %order_number%')),
        );
    }

    if (module_exists('uc_payment')) {
        $form['yoomoney_api_kassa_tab']['yoomoney_api_description_template'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Transaction data'),
            '#description'   => t("Full description of the transaction that the user will see during the checkout process. You can find it in your YooKassa Merchant Profile. For example, \"Payment for order No. 72 by user@yoomoney.ru\". Limitations: no more than 128 symbols"),
            '#default_value' => variable_get('yoomoney_api_description_template', t('Payment for order No. %order_id%')),
            '#size'          => 60,
        );
    }

    /*********** Hold mode ***********/
    $form['yoomoney_api_kassa_tab']['yoomoney_api_enable_hold_mode'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Включить отложенную оплату'),
        '#description'   => t('Если опция включена, платежи с карт проходят в 2 этапа: у клиента сумма замораживается, и вам вручную нужно подтвердить её списание – через панель администратора. <a href="https://yookassa.ru/features/" target="_blank">Подробное описание Холдирования.</a>'),
        '#default_value' => variable_get('yoomoney_api_enable_hold_mode', 0),
    );

    /*********** tax edit ****************/

    if (module_exists('commerce_tax') && module_exists('commerce_tax_ui')) {
        $form['yoomoney_api_kassa_tab']['yoomoney_api_send_check'] = array(
            '#type'          => 'radios',
            '#title'         => t('Transmit details for receipts to YooKassa (Federal Law No. 54-FZ)'),
            '#options'       => array(
                '1' => t('Enable'),
                '0' => t('Disable'),
            ),
            '#default_value' => variable_get('yoomoney_api_send_check', '0'),
        );

        $taxes = commerce_tax_ui_commerce_tax_rate_info();

        if ($taxes && variable_get('yoomoney_api_send_check')) {

            $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_default'] = array(
                '#markup' => '<div> <label style="width: 200px;float: left;padding-top: 10px;">'.t('Default tax rate').'</label>',
            );
            $form['yoomoney_api_kassa_tab']['yoomoney_api_kassa_tax_default']     = array(
                '#type'          => 'select',
                '#title_display' => false,
                '#options'       => array(
                    1 => t('Without VAT'),
                    2 => t('0%'),
                    3 => t('10%'),
                    4 => t('20%'),
                    5 => t('Applicable rate 10/110'),
                    6 => t('Applicable rate 20/120'),
                ),
                '#default_value' => variable_get('yoomoney_api_kassa_tax_default', YOOMONEY_API_DEFAULT_TAX_RATE_ID),
            );

            $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_default_end'] = array(
                '#markup' => '</div>',
            );

            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax'] = array(
                '#markup' => '<label>Сопоставьте ставки</label>',
            );

            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax_start'] = array(
                '#markup' => '<div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_shop_tax']          = array(
                '#markup' => '<div style="float: left;width: 200px;">'. t('Rate at your store').'</div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_kassa_tax']         = array(
                '#markup' => '<div>'.t('Rate for the receipt sent to the tax service').'</div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax_end']   = array(
                '#markup' => '</div>',
            );

            foreach ($taxes as $tax) {
                $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_'.$tax['name']] = array(
                    '#markup' => '<div> <label style="width: 200px;float: left;padding-top: 10px;">'.$tax['title'].'</label>',
                );
                $form['yoomoney_api_kassa_tab']['yoomoney_api_kassa_tax_'.$tax['name']]     = array(
                    '#type'          => 'select',
                    '#title_display' => false,
                    '#options'       => array(
                        1 => t('Without VAT'),
                        2 => t('0%'),
                        3 => t('10%'),
                        4 => t('20%'),
                        5 => t('Applicable rate 10/110'),
                        6 => t('Applicable rate 20/120'),
                    ),
                    '#default_value' => variable_get('yoomoney_api_kassa_tax_'.$tax['name'], YOOMONEY_API_DEFAULT_TAX_RATE_ID),
                );

                $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_'.$tax['name'].'_end'] = array(
                    '#markup' => '</div>',
                );
            }

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_payment_mode'] = array(
                '#type' => 'select',
                '#title' => t('Признак способа расчета'),
                '#options' => array(
                    'full_prepayment'    => 'Полная предоплата (full_prepayment)',
                    'partial_prepayment' => 'Частичная предоплата (partial_prepayment)',
                    'advance'            => 'Аванс (advance)',
                    'full_payment'       => 'Полный расчет (full_payment)',
                    'partial_payment'    => 'Частичный расчет и кредит (partial_payment)',
                    'credit'             => 'Кредит (credit)',
                    'credit_payment'     => 'Выплата по кредиту (credit_payment)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_payment_mode'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_payment_subject'] = array(
                '#type' => 'select',
                '#title' => t('Признак предмета расчета'),
                '#options' => array(
                    'commodity'             => 'Товар (commodity)',
                    'excise'                => 'Подакцизный товар (excise)',
                    'job'                   => 'Работа (job)',
                    'service'               => 'Услуга (service)',
                    'gambling_bet'          => 'Ставка в азартной игре (gambling_bet)',
                    'gambling_prize'        => 'Выигрыш в азартной игре (gambling_prize)',
                    'lottery'               => 'Лотерейный билет (lottery)',
                    'lottery_prize'         => 'Выигрыш в лотерею (lottery_prize)',
                    'intellectual_activity' => 'Результаты интеллектуальной деятельности (intellectual_activity)',
                    'payment'               => 'Платеж (payment)',
                    'agent_commission'      => 'Агентское вознаграждение (agent_commission)',
                    'composite'             => 'Несколько вариантов (composite)',
                    'another'               => 'Другое (another)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_payment_subject'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_delivery_payment_mode'] = array(
                '#type' => 'select',
                '#title' => t('Признак способа расчета для доставки'),
                '#options' => array(
                    'full_prepayment'    => 'Полная предоплата (full_prepayment)',
                    'partial_prepayment' => 'Частичная предоплата (partial_prepayment)',
                    'advance'            => 'Аванс (advance)',
                    'full_payment'       => 'Полный расчет (full_payment)',
                    'partial_payment'    => 'Частичный расчет и кредит (partial_payment)',
                    'credit'             => 'Кредит (credit)',
                    'credit_payment'     => 'Выплата по кредиту (credit_payment)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_delivery_payment_mode'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_delivery_payment_subject'] = array(
                '#type' => 'select',
                '#title' => t('Признак предмета расчета для доставки'),
                '#options' => array(
                    'commodity'             => 'Товар (commodity)',
                    'excise'                => 'Подакцизный товар (excise)',
                    'job'                   => 'Работа (job)',
                    'service'               => 'Услуга (service)',
                    'gambling_bet'          => 'Ставка в азартной игре (gambling_bet)',
                    'gambling_prize'        => 'Выигрыш в азартной игре (gambling_prize)',
                    'lottery'               => 'Лотерейный билет (lottery)',
                    'lottery_prize'         => 'Выигрыш в лотерею (lottery_prize)',
                    'intellectual_activity' => 'Результаты интеллектуальной деятельности (intellectual_activity)',
                    'payment'               => 'Платеж (payment)',
                    'agent_commission'      => 'Агентское вознаграждение (agent_commission)',
                    'composite'             => 'Несколько вариантов (composite)',
                    'another'               => 'Другое (another)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_delivery_payment_subject'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_api_send_second_receipt'] = array(
                '#type'          => 'radios',
                '#title'         => t('Second receipt'),
                '#options'       => array(
                    '1' => t('Enable'),
                    '0' => t('Disable'),
                ),
                '#default_value' => variable_get('yoomoney_api_send_second_receipt', '0'),
            );

            $statuses = array();

            foreach (commerce_order_state_get_title() as $name => $title) {
                foreach (commerce_order_statuses(array('state' => $name)) as $order_status) {
                    // Only include enabled statues and the current order status.
                    if (!empty($order_status['status'])) {
                        $statuses[check_plain($title)][$order_status['name']] = check_plain($order_status['title']);
                    }
                }
            }

            $form['yoomoney_api_kassa_tab']['yoomoney_api_second_receipt_status'] = array(
                '#type' => 'select',
                '#title' => t('Generate a second check when the order goes into status'),
                '#options' => $statuses,
                '#default_value' => variable_get('yoomoney_api_second_receipt_status'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                        ':input[name="yoomoney_api_send_second_receipt"]' => array('value' => '0'),
                    ),
                )
            );
        }
    }

    /*********** tax edit****************/

    if (module_exists('uc_taxes')) {
        $form['yoomoney_api_kassa_tab']['yoomoney_api_send_check'] = array(
            '#type'          => 'radios',
            '#title'         => t('Transmit details for receipts to YooKassa (Federal Law No. 54-FZ)'),
            '#options'       => array(
                '1' => t('Enable'),
                '0' => t('Disable'),
            ),
            '#default_value' => variable_get('yoomoney_api_send_check', '0'),
        );

        $taxes = uc_taxes_rate_load();

        if ($taxes && variable_get('yoomoney_api_send_check')) {
            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax'] = array(
                '#markup' => '<label>Сопоставьте ставки</label>',
            );

            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax_start'] = array(
                '#markup' => '<div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_shop_tax']          = array(
                '#markup' => '<div style="float: left;width: 200px;">'.t('Rate at your store').'</div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_kassa_tax']         = array(
                '#markup' => '<div>'.t('Rate for the receipt sent to the tax service').'</div>',
            );
            $form['yoomoney_api_kassa_tab']['lbl_wrapper_tax_end']   = array(
                '#markup' => '</div>',
            );

            foreach ($taxes as $tax) {

                $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_'.$tax->id] = array(
                    '#markup' => '<div> <label style="width: 200px;float: left;padding-top: 10px;">'.$tax->name.'</label>',
                );

                $form['yoomoney_api_kassa_tab']['yoomoney_api_kassa_tax_'.$tax->id] = array(
                    '#type'          => 'select',
                    '#label'         => false,
                    '#options'       => array(
                        1 => t('Without VAT'),
                        2 => t('0%'),
                        3 => t('10%'),
                        4 => t('20%'),
                        5 => t('Applicable rate 10/110'),
                        6 => t('Applicable rate 20/120'),
                    ),
                    '#default_value' => variable_get('yoomoney_api_kassa_tax_'.$tax->id, 1),
                    '#description'   => '',
                );

                $form['yoomoney_api_kassa_tab']['lbl_yoomoney_api_kassa_tax_'.$tax->id.'_end'] = array(
                    '#markup' => '</div>',
                );
            }

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_payment_mode'] = array(
                '#type' => 'select',
                '#title' => t('Признак способа расчета'),
                '#options' => array(
                    'full_prepayment'    => 'Полная предоплата (full_prepayment)',
                    'partial_prepayment' => 'Частичная предоплата (partial_prepayment)',
                    'advance'            => 'Аванс (advance)',
                    'full_payment'       => 'Полный расчет (full_payment)',
                    'partial_payment'    => 'Частичный расчет и кредит (partial_payment)',
                    'credit'             => 'Кредит (credit)',
                    'credit_payment'     => 'Выплата по кредиту (credit_payment)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_payment_mode'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_payment_subject'] = array(
                '#type' => 'select',
                '#title' => t('Признак предмета расчета'),
                '#options' => array(
                    'commodity'             => 'Товар (commodity)',
                    'excise'                => 'Подакцизный товар (excise)',
                    'job'                   => 'Работа (job)',
                    'service'               => 'Услуга (service)',
                    'gambling_bet'          => 'Ставка в азартной игре (gambling_bet)',
                    'gambling_prize'        => 'Выигрыш в азартной игре (gambling_prize)',
                    'lottery'               => 'Лотерейный билет (lottery)',
                    'lottery_prize'         => 'Выигрыш в лотерею (lottery_prize)',
                    'intellectual_activity' => 'Результаты интеллектуальной деятельности (intellectual_activity)',
                    'payment'               => 'Платеж (payment)',
                    'agent_commission'      => 'Агентское вознаграждение (agent_commission)',
                    'composite'             => 'Несколько вариантов (composite)',
                    'another'               => 'Другое (another)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_payment_subject'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_delivery_payment_mode'] = array(
                '#type' => 'select',
                '#title' => t('Признак способа расчета для доставки'),
                '#options' => array(
                    'full_prepayment'    => 'Полная предоплата (full_prepayment)',
                    'partial_prepayment' => 'Частичная предоплата (partial_prepayment)',
                    'advance'            => 'Аванс (advance)',
                    'full_payment'       => 'Полный расчет (full_payment)',
                    'partial_payment'    => 'Частичный расчет и кредит (partial_payment)',
                    'credit'             => 'Кредит (credit)',
                    'credit_payment'     => 'Выплата по кредиту (credit_payment)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_delivery_payment_mode'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_kassa_delivery_payment_subject'] = array(
                '#type' => 'select',
                '#title' => t('Признак предмета расчета для доставки'),
                '#options' => array(
                    'commodity'             => 'Товар (commodity)',
                    'excise'                => 'Подакцизный товар (excise)',
                    'job'                   => 'Работа (job)',
                    'service'               => 'Услуга (service)',
                    'gambling_bet'          => 'Ставка в азартной игре (gambling_bet)',
                    'gambling_prize'        => 'Выигрыш в азартной игре (gambling_prize)',
                    'lottery'               => 'Лотерейный билет (lottery)',
                    'lottery_prize'         => 'Выигрыш в лотерею (lottery_prize)',
                    'intellectual_activity' => 'Результаты интеллектуальной деятельности (intellectual_activity)',
                    'payment'               => 'Платеж (payment)',
                    'agent_commission'      => 'Агентское вознаграждение (agent_commission)',
                    'composite'             => 'Несколько вариантов (composite)',
                    'another'               => 'Другое (another)',
                ),
                '#default_value' => variable_get('yoomoney_kassa_delivery_payment_subject'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                    ),
                )
            );

            $form['yoomoney_api_kassa_tab']['yoomoney_api_send_second_receipt'] = array(
                '#type'          => 'radios',
                '#title'         => t('Second receipt'),
                '#options'       => array(
                    '1' => t('Enable'),
                    '0' => t('Disable'),
                ),
                '#default_value' => variable_get('yoomoney_api_send_second_receipt', '0'),
            );

            $statuses = array();
            foreach (uc_order_status_list() as $status) {
                $statuses[$status['id']] = $status['title'];
            }

            $form['yoomoney_api_kassa_tab']['yoomoney_api_second_receipt_status'] = array(
                '#type' => 'select',
                '#title' => t('Generate a second check when the order goes into status'),
                '#options' => $statuses,
                '#default_value' => variable_get('yoomoney_api_second_receipt_status'),
                '#states' => array(
                    'invisible' => array(
                        ':input[name="yoomoney_send_check"]' => array('value' => '0'),
                        ':input[name="yoomoney_api_send_second_receipt"]' => array('value' => '0'),
                    ),
                )
            );
        }
    }

    /*********** tax edit****************/
    if (module_exists('commerce')) {
        $form['yoomoney_api_kassa_tab']['notifyUrl'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Address for notifications'),
            '#attributes'    => array('disabled' => 'disabled'),
            '#description'   => t('Only required if YooKassa\'s specialists ask for it'),
            '#default_value' => url('yoomoney_api/commerce/notify', array('absolute' => true)),
        );
    }

    if (module_exists('uc_payment')) {
        $form['yoomoney_api_kassa_tab']['notifyUrl'] = array(
            '#type'          => 'textfield',
            '#title'         => t('Address for notifications'),
            '#attributes'    => array('disabled' => 'disabled'),
            '#description'   => t('Only required if YooKassa\'s specialists ask for it'),
            '#default_value' => url('yoomoney_api/ubercart/notify', array('absolute' => true)),
        );
    }

    $form['yoomoney_api_kassa_tab']['yoomoney_api_debug'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Record of debugging information'),
        '#description'   => t('Only required if YooKassa\'s specialists ask for it'),
        '#default_value' => variable_get('yoomoney_api_debug', 0),
    );

    if (variable_get('yoomoney_api_debug')) {
        $form['yoomoney_api_kassa_tab']['my_button']['#type']                     = 'button';
        $form['yoomoney_api_kassa_tab']['my_button']['#value']                    = t('Download the log');
        $form['yoomoney_api_kassa_tab']['my_button']['#submit']                   = array('yoomoney_api_log_handler');
        $form['yoomoney_api_kassa_tab']['my_button']['#executes_submit_callback'] = true;
    }

    /* ---- */
    $form['yoomoney_api_p2p_tab']                            = array(
        '#type'   => 'fieldset',
        '#title'  => t('YooMoney'),
        '#group'  => 'vertical_tabs',
        '#weight' => 30,
    );
    $form['yoomoney_api_p2p_tab']['intro']                   = array(
        '#markup' => t('To start operating the module, you need to <a href="https://yoomoney.ru/new" target="_blank">start a YooMoney wallet</a> and <a href="https://yoomoney.ru/transfer/myservices/http-notification" target="_blank">register the app</a> at the YooMoney website'),
    );
    $form['yoomoney_api_p2p_tab']['yoomoney_api_p2p']         = array(
        '#type'          => 'checkbox',
        '#title'         => t('Enable payment acceptance to the YooMoney Wallet'),
        '#default_value' => variable_get('yoomoney_api_p2p', 0),
    );
    $form['yoomoney_api_p2p_tab']['lbl_redirectUrl']         = array(
        '#type'          => 'textfield',
        '#title'         => t('RedirectURL'),
        '#attributes'    => array('disabled' => 'disabled'),
        '#default_value' => url('yoomoney_api/check_wallet', array('absolute' => true)),
        '#description'   => t(
            "Copy this link to the upper field at the <a href='https://yoomoney.ru/transfer/myservices/http-notification' target='_blank'>HTTP notifications page</a>"
        ),
    );
    $form['yoomoney_api_p2p_tab']['lbl_setting']             = array(
        '#markup' => t('<h3>Payment acceptance settings</h3>'),
    );
    $form['yoomoney_api_p2p_tab']['yoomoney_api_receiver']    = array(
        '#type'          => 'textfield',
        '#title'         => t('Wallet number'),
        '#description'   => t('Your YooMoney wallet number.'),
        '#default_value' => variable_get('yoomoney_api_receiver', ''),
        '#size'          => 14,
        //'#states' => $p2p_states,
    );
    $form['yoomoney_api_p2p_tab']['yoomoney_secret']          = array(
        '#type'          => 'textfield',
        '#title'         => t("Codeword"),
        '#description'   => t("Codeword"),
        '#default_value' => variable_get('yoomoney_secret', ''),
        '#size'          => 17,
    );
    $form['yoomoney_api_p2p_tab']['yoomoney_api_formcomment'] = array(
        '#type'          => 'textfield',
        '#title'         => t("Payment comment"),
        '#description'   => t('This title will be shown to the customer'),
        '#default_value' => variable_get('yoomoney_api_formcomment', $base_url),
        //'#states' => $p2p_states,
    );

    // footer settings
    $form['yoomoney_api_texts']                             = array(
        '#type'   => 'fieldset',
        '#title'  => t('Text for success and fail payment pages'),
        '#weight' => 40,
    );
    $success                                               = variable_get(
        'yoomoney_api_success_text',
        array('value' => '', 'format' => '')
    );
    $form['yoomoney_api_texts']['yoomoney_api_success_text'] = array(
        '#type'          => 'text_format',
        '#title'         => t('Text for success page'),
        '#default_value' => $success['value'] ? $success['value'] : '',
        '#format'        => $success['format'] ? $success['format'] : '',
    );
    $fail                                                  = variable_get(
        'yoomoney_api_fail_text',
        array('value' => '', 'format' => '')
    );
    $form['yoomoney_api_texts']['yoomoney_api_fail_text']    = array(
        '#type'          => 'text_format',
        '#title'         => t('Text for fail page'),
        '#default_value' => $fail['value'] ? $fail['value'] : '',
        '#format'        => $fail['format'] ? $fail['format'] : '',
    );

    return system_settings_form($form);
}

/**
 * @param array $form
 * @param array $form_state
 */
function yoomoney_api_admin_settings_validate($form, &$form_state)
{
    $checkboxs = $form_state['values']['yoomoney_api_payment_method'];

    if ($form_state['values']['yoomoney_api_shop'] == $form_state['values']['yoomoney_api_p2p'] && $form_state['values']['yoomoney_api_shop']) {
        $form_state['values']['yoomoney_api_shop'] = 1;
        $form_state['values']['yoomoney_api_p2p']  = 0;
        drupal_set_message(
            t('Невозможно одновременно включить оплату через ЮMoney и ЮKassa'),
            'warning'
        );
        form_set_error('yoomoney_api_shop');
        form_set_error('yoomoney_api_p2p');
    }

    if ($form_state['values']['yoomoney_api_shop'] == 1) {
        if ($form_state['values']['yoomoney_api_paymode'] == 'shop' && !count(array_filter($checkboxs))) {
            form_set_error(
                'yoomoney_api_payment_method',
                'Не выбрано ни одного способа оплаты'
            );
        }

        $valid = true;

        if (!preg_match('/^\d+$/i', $form_state['values']['yoomoney_api_shop_id'])) {
            form_set_error(
                'yoomoney_api_shop_id',
                t('Such shopId does not exist. Please copy this parameter under your <a href=\"https://yoomoney.ru/joinups\">YooKassa\'s Merchant Profile</a> (at the top of any page)')
            );
            $valid = false;
        }

        if (!preg_match('/^test_.*|live_.*$/i', $form_state['values']['yoomoney_api_secret'])) {
            form_set_error(
                'yoomoney_api_secret',
                t('Such secret key does not exist. If you are sure you copied the key correctly, this means it does not work for some reason. Issue and activate the key again—under your <a href=\"https://yoomoney.ru/joinups\">Merchant Profile</a>')
            );
            $valid = false;
        }

        if ($valid) {
            $shopId = $form_state['values']['yoomoney_api_shop_id'];
            $password = $form_state['values']['yoomoney_api_secret'];
            if (!yoomoney_api_admin_settings_test_connection($shopId, $password)) {
                drupal_set_message(
                    t('Проверьте shopId и Секретный ключ — где-то есть ошибка. А лучше скопируйте их прямо из'
                        . ' <a href="https://yookassa.ru/my" target="_blank">личного кабинета ЮKassa</a>'),
                    'warning'
                );
                form_set_error('yoomoney_api_shop_id', '');
                form_set_error('yoomoney_api_secret', '');
            }
        }
    }

    if ($form_state['values']['yoomoney_api_p2p'] == 1) {
        if (empty($form_state['values']['yoomoney_api_receiver'])) {
            form_set_error(
                'yoomoney_api_receiver',
                t('Wallet number field is missing')
            );
        }

        if (empty($form_state['values']['yoomoney_secret'])) {
            form_set_error(
                'yoomoney_secret',
                t('Codeword field is missing')
            );
        }

        if (empty($form_state['values']['yoomoney_api_formcomment'])) {
            form_set_error(
                'yoomoney_api_formcomment',
                t('Payment comment field is missing')
            );
        }

    }

}

function yoomoney_api_form_alter(&$form, &$form_state, $form_id)
{
    YooMoneyLogger::error($form_id);
}

function yoomoney_api_admin_settings_alter(&$form, &$form_state)
{

}


function yoomoney_api_log_handler()
{
    $path      = variable_get('file_public_path', conf_path().'/files');
    $filePath  = $path.'/ym-checkout-debug.log';
    $file      = fopen($filePath, 'r');
    $file_size = filesize($filePath);

    header("Expires: 0");
    header("Cache-Control: no-cache, no-store, must-revalidate");
    header('Cache-Control: pre-check=0, post-check=0, max-age=0', false);
    header("Pragma: no-cache");
    header("Content-Disposition:attachment; filename=debug.log");
    header("Content-Type: application/force-download");
    while (!feof($file)) {
        $buffer = fread($file, 2048);
        echo $buffer;
        flush();
    }
    fclose($file);
    die();
}

function getUcOrderStatuses()
{
    $query = db_query("SELECT * FROM {uc_order_statuses}");

    return $query->fetchAllKeyed();
}

function yoomoney_api_admin_remove_duplicate_messages($type)
{
    $stack = array();
    if (isset($_SESSION['messages'][$type])) {
        foreach ($_SESSION['messages'][$type] as $k => $msg) {
            if (in_array($msg, $stack)) {
                // Message is already set - remove it
                unset($_SESSION['messages'][$type][$k]);
            } else {
                // Add message to stack
                $stack[] = $msg;
            }
        }
    }
}

function yoomoney_api_admin_settings_test_connection($shopId, $password)
{
    require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'YooMoneyLogger.php';
    require_once dirname(__FILE__) .DIRECTORY_SEPARATOR.'vendor'.DIRECTORY_SEPARATOR.'autoload.php';

    $apiClient    = new \YooKassa\Client();
    $apiClient->setAuth($shopId, $password);
    $apiClient->setLogger(new YooMoneyLogger());

    try {
        $payment = $apiClient->getPaymentInfo('00000000-0000-0000-0000-000000000001');
    } catch (\YooKassa\Common\Exceptions\NotFoundException $e) {
        return true;
    } catch (\Exception $e) {
        return false;
    }
    return true;
}