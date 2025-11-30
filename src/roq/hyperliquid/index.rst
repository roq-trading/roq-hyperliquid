.. _roq-hyperliquid:
.. _roq-hyperliquid-v5:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


roq-hyperliquid
===============

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-hyperliquid

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-hyperliquid


Supports
~~~~~~~~

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |cross-mark|
        -
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |cross-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.

   |footnote-1| The exchange protocol does not support streaming updates for reference data and market status.


Using
-----

.. code-block:: shell

   $ roq-hyperliquid [FLAGS]


.. _roq-hyperliquid-flags:

Flags
~~~~~

.. code-block:: shell

   $ roq-hyperliquid --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: MBP

   .. include:: flags/mbp.rstinc

.. tab:: Request

   .. include:: flags/request.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
~~~~~~~~~~~~

.. tab:: Prod

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-hyperliquid/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-hyperliquid/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell


Configuration
~~~~~~~~~~~~~

.. code-block:: shell

   $ --flagfile $CONDA_PREFIX/share/roq-hyperliquid/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       -
       -

     * - :code:`activeAssetCtx`
       - :code:`openInterest`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_INTEREST <roq::StatisticsType::OPEN_INTEREST>`

     * - :code:`activeAssetCtx`
       - :code:`dayNtlVlm`
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOLUME <roq::StatisticsType::TRADE_VOLUME>`

     * - :code:`activeAssetCtx`
       - :code:`markPx`
       - |right-double-arrow|
       - :cpp:enumerator:`SETTLEMENT_PRICE <roq::StatisticsType::SETTLEMENT_PRICE>`

     * - :code:`activeAssetCtx`
       - :code:`funding`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`


Order Management
~~~~~~~~~~~~~~~~


Comments
~~~~~~~~

* The gateway can not simultaneously support all product categories due to
  overlapping symbol names, e.g. BTCUSDT being both spot and linear.
  For this reason, the :code:`--api` flag controls the product category and, if
  necessary, the :code:`--name` or :code:`--exchange` flags must be configured
  to appropriately differentitate the sources.

* The :code:`order` channel doesn't give us any information about last traded,
  only the aggregate fields (traded / remaining / average price) are available.
  The last trade price/quantity fields are therefore estimated.

  .. note::
     The :code:`execution` channel will independently report the fills.

* :code:`TopOfBook` is based on :code:`orderbook.1` for spot and :code:`tickers`
  for all other categories.


References
----------


Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


Exchange
~~~~~~~~

* `Website <https://www.hyperliquid.com/>`__
* `Documentation <https://hyperliquid-exchange.github.io/docs/v5/intro>`__
